#include <valgrind/helgrind.h>
#include "yaps_rcu.h"
#include "yaps_rcu_private.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define CELL_UNDEF SSIZE_MAX
#define TAKE_SPINLOCK(mutex) {                               \
        atomic_uint zero = 0;                                \
        while (! atomic_compare_exchange_strong(&(mutex),    \
                                                &zero, 1)) { \
            zero = 0;                                        \
        }                                                    \
    }

#define RELEASE_SPINLOCK(mutex) { \
        mutex = 0;                \
    }

#define GET_VARIABLE_POOL_CELL(v, index) \
    &(v->pool->pool[index])

size_t alloc(cellpool_t *pool)
{
    size_t position;
    _Bool trueBool = 1;
    cell_ptr_t result;
    atomic_uint zero = 0;
    do {
        position = atomic_fetch_add(&(pool->next), 1) % pool->poolSize;
        result = &(pool->pool[position]);
        zero = 0;
    } while (! atomic_compare_exchange_strong(&(result->rctr), &zero, 1)); // Reader grace period
    return position;
}

void set(variable_t *v, int newValue)
{
    atomic_uint zero = 0;
    struct variable_private *vp = v->_p;
    cell_t *c;
    size_t nci = alloc(v->pool);
    cell_t *nc = GET_VARIABLE_POOL_CELL(v, nci);
    size_t cIndex = atomic_fetch_add(&(vp->next), 1) % vp->cSize;

    nc->value = newValue;

    // per-variable mutex
    TAKE_SPINLOCK(vp->mutex);

    if (vp->c[cIndex] != CELL_UNDEF) {
        c = GET_VARIABLE_POOL_CELL(v, vp->c[cIndex]);
        // Mark as released
        atomic_fetch_sub(&(c->rctr), 1); // End of protected section
    }
    vp->c[vp->next % vp->cSize] = nci;

    // Prevent new value from being released
    atomic_fetch_add(&(nc->rctr), 1);

    RELEASE_SPINLOCK(vp->mutex);

    // Dispatch to notified clients
    // Release new value
    atomic_fetch_sub(&(nc->rctr), 1);
}

int get(variable_t *v)
{
    atomic_uint zero = 0;
    struct variable_private *vp = v->_p;
    cell_t *c;

    // per-variable mutex
    TAKE_SPINLOCK(vp->mutex);

    c = GET_VARIABLE_POOL_CELL(v, vp->c[0]);
    atomic_fetch_add(&(c->rctr), 1); // Protect readers

    RELEASE_SPINLOCK(vp->mutex);

    int result = c->value;
    atomic_fetch_sub(&(c->rctr), 1); // End of protected section
    return result;
}

void initVariable(variable_t *v,
                  size_t nmemb)
{
    struct variable_private *vp = v->_p;
    vp->cSize = nmemb;
    vp->next = ATOMIC_VAR_INIT(0);
    for (size_t i = 0 ; i < nmemb ; ++i) {
        vp->c[i] = CELL_UNDEF;
    }
    vp->mutex = ATOMIC_VAR_INIT(0);
}

variable_t *mapVariable(cellpool_t *pool, void *vmemory)
{
    variable_t *v = calloc(1, sizeof(variable_t));
    v->pool = pool;
    v->_p = vmemory;
    return v;
}

variable_t *allocInitVariable(cellpool_t *pool, size_t depth)
{
    struct variable_private *vp = calloc(1, sizeof(struct variable_private) + depth*sizeof(size_t));
    variable_t *v = mapVariable(pool, vp);
    initVariable(v, depth);
    return v;
}

void initPool(cellpool_t *pool, size_t nmemb)
{
    //pool->pool = cell_memory;
    pool->poolSize = nmemb;
    pool->next = ATOMIC_VAR_INIT(0);
    for (size_t i = 0 ; i < nmemb ; ++i) {
        pool->pool[i].rctr = ATOMIC_VAR_INIT(0);
    }
}

cellpool_t *allocInitPool(size_t nmemb)
{
    cellpool_t *pool = calloc(1, sizeof(cellpool_t) + nmemb*sizeof(cell_t));
    //cell_ptr_t cell_memory = calloc(nmemb, sizeof(cell_t));
    initPool(pool, nmemb);
    return pool;
}

size_t pool_getMemSizeOf(cellpool_t *pool)
{
    return sizeof(cellpool_t) + (pool->poolSize)*sizeof(cell_t);
}

void dumpPool(cellpool_t *pool)
{
    for (size_t i = 0 ; i < pool->poolSize ; ++i) {
        printf("dump: pool=%zu, rctr=%d\n", i, pool->pool[i].rctr);
    }
}
