#include <valgrind/helgrind.h>
#include "yaps_rcu.h"
#include "yaps_rcu_private.h"
#include <stdlib.h>
#include <stdio.h>

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

cell_ptr_t alloc(cellpool_t *pool)
{
    size_t position;
    _Bool trueBool = 1;
    cell_ptr_t result;
    atomic_uint zero = 0;
    do {
        position = atomic_fetch_add(&(pool->next), 1) % pool->poolSize;
        result = &((&(pool->pool))[position]);
        zero = 0;
    } while (! atomic_compare_exchange_strong(&(result->rctr), &zero, 1)); // Reader grace period
    return result;
}

void set(variable_t *v, int newValue)
{
    atomic_uint zero = 0;
    atomic_cell_ptr_t c;
    atomic_cell_ptr_t nc = alloc(v->pool);

    nc->value = newValue;

    // per-variable mutex
    TAKE_SPINLOCK(v->mutex);

    atomic_store(&c, v->c); // should be atomic?
    if (c != NULL) {
        // Mark as released
        atomic_fetch_sub(&(c->rctr), 1); // End of protected section
    }
    atomic_store(&(v->c), nc); // must be atomic

    // Prevent new value from being released
    atomic_fetch_add(&(nc->rctr), 1);

    RELEASE_SPINLOCK(v->mutex);

    // Dispatch to notified clients
    // Release new value
    atomic_fetch_sub(&(nc->rctr), 1);
}

int get(variable_t *v)
{
    atomic_uint zero = 0;
    atomic_cell_ptr_t c;

    // per-variable mutex
    TAKE_SPINLOCK(v->mutex);

    atomic_store(&c, v->c); // must be atomic
    atomic_fetch_add(&(c->rctr), 1); // Protect readers

    RELEASE_SPINLOCK(v->mutex);

    int result = c->value;
    atomic_fetch_sub(&(c->rctr), 1); // End of protected section
    return result;
}

void initVariable(variable_t *v, cellpool_t *pool)
{
    v->c = ATOMIC_VAR_INIT(NULL);
    v->pool = pool;
    v->mutex = ATOMIC_VAR_INIT(0);
}

void initPool(cellpool_t *pool, size_t nmemb)
{
    //pool->pool = cell_memory;
    pool->poolSize = nmemb;
    pool->next = ATOMIC_VAR_INIT(0);
    for (size_t i = 0 ; i < nmemb ; ++i) {
        (&pool->pool)[i].rctr = ATOMIC_VAR_INIT(0);
        (&pool->pool)[i].obsolete = ATOMIC_VAR_INIT(1);
    }
}

cellpool_t *allocInitPool(size_t nmemb)
{
    cellpool_t *pool = calloc(1, sizeof(cellpool_t) + (nmemb-1)*sizeof(cell_t));
    //cell_ptr_t cell_memory = calloc(nmemb, sizeof(cell_t));
    initPool(pool, nmemb);
    return pool;
}

void dumpPool(cellpool_t *pool)
{
    for (size_t i = 0 ; i < pool->poolSize ; ++i) {
        printf("dump: pool=%zu, rctr=%d\n", i, (&pool->pool)[i].rctr);
    }
}
