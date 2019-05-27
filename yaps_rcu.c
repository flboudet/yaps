#include <valgrind/helgrind.h>
#include "yaps_rcu.h"
#include <stdlib.h>
#include <stdio.h>

cell_ptr_t alloc(cellpool_t *pool)
{
    size_t position;
    _Bool trueBool = 1;
    cell_ptr_t result;
    atomic_uint zero = 0;
    do {
        position = atomic_fetch_add(&(pool->next), 1) % pool->poolSize;
        result = &(pool->pool[position]);
        //printf("iterate: pool=%zu, rctr=%d\n", position, result->rctr);
        //ANNOTATE_HAPPENS_BEFORE(&(result->rctr));
        //} while (! (result->rctr == 0));
        //result->rctr = 2;
        zero = 0;
    } while (! atomic_compare_exchange_strong(&(result->rctr), &zero, 1)); // Reader grace period

    //printf("ended iterate: pool=%d, rctr=%d\n", position, result->rctr);
    //ANNOTATE_HAPPENS_AFTER(&(result->rctr));
    //ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(&(result->rctr));
    //atomic_store(&(result->rctr), 1);
    return result;
}

void set(variable_t *v, int newValue)
{
    atomic_cell_ptr_t c;
    atomic_store(&c, v->c); // should be atomic?
    atomic_cell_ptr_t nc = alloc(v->pool);
    nc->value = newValue;
    if (c != NULL) {
        //printf("Cell will be released\n");
        // Mark as released
        //VALGRIND_HG_CLEAN_MEMORY(&(nc->value), sizeof(nc->value));
        atomic_fetch_sub(&(c->rctr), 1); // End of protected section
        //atomic_store(&(c->obsolete), 1);
    }
    atomic_store(&(v->c), nc); // must be atomic
}

int get(variable_t *v)
{
    atomic_cell_ptr_t c;
    atomic_store(&c, v->c); // must be atomic
    atomic_fetch_add(&(c->rctr), 1); // Protect readers

    int result = c->value;
    atomic_fetch_sub(&(c->rctr), 1); // End of protected section
    return result;
}

void initVariable(variable_t *v, cellpool_t *pool)
{
    v->c = ATOMIC_VAR_INIT(NULL);
    v->pool = pool;
    //printf("cell: %p\n", v->c);
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

void dumpPool(cellpool_t *pool)
{
    for (size_t i = 0 ; i < pool->poolSize ; ++i) {
        printf("dump: pool=%zu, rctr=%d\n", i, pool->pool[i].rctr);
    }
}
