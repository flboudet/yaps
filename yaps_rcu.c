#include "yaps_rcu.h"
#include <stdlib.h>

cell_ptr_t alloc(cellpool_t *pool)
{
    size_t position;
    _Bool trueBool = 1;
    cell_ptr_t result;
    do {
        position = atomic_fetch_add(&(pool->next), 1) % pool->poolSize;
        result = &(pool->pool[position]);
        if (atomic_load(&(result->rctr))) { // Reader grace period
            continue;
        }
    } while (atomic_compare_exchange_strong(&(result->obsolete), &trueBool, 0) == 0);
    return result;
}

void set(variable_t *v, int newValue)
{
    atomic_cell_ptr_t c;
    atomic_store(&c, v->c); // should be atomic?
    atomic_cell_ptr_t nc = alloc(v->pool);
    nc->value = newValue;
    nc->obsolete = 0;
    if (c != NULL) {
        // Mark as released
        atomic_store(&(c->obsolete), 1);
    }
    atomic_store(&(v->c), nc); // must be atomic
}

int get(variable_t *v)
{
    atomic_cell_ptr_t c;
    atomic_fetch_add(&(v->c->rctr), 1); // Protect readers
    atomic_store(&c, v->c); // must be atomic
    int result = c->value;
    atomic_fetch_sub(&(v->c->rctr), 1); // End of protected section
    return result;
}

void initVariable(variable_t *v, cellpool_t *pool)
{
    v->c = ATOMIC_VAR_INIT(NULL);
    v->pool = pool;
}

void initPool(cellpool_t *pool, size_t nmemb)
{
    pool->pool = calloc(nmemb, sizeof(cell_t));
    pool->poolSize = nmemb;
    pool->next = ATOMIC_VAR_INIT(0);
    for (size_t i = 0 ; i < nmemb ; ++i) {
        pool->pool[i].rctr = ATOMIC_VAR_INIT(0);
        pool->pool[i].obsolete = ATOMIC_VAR_INIT(1);
    }
}
