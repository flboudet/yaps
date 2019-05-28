#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
using namespace std;
# define _Atomic(X) std::atomic< X >
#endif

#include <stddef.h>
#include <pthread.h>

struct cell {
    int value;
    atomic_bool obsolete;
    atomic_uint rctr;
};

typedef struct cell cell_t;
typedef cell_t * cell_ptr_t;
typedef cell_ptr_t atomic_cell_ptr_t;

struct cellpool {
    cell_ptr_t pool;
    size_t poolSize;
    atomic_size_t next;
    atomic_uint mutex;
    pthread_mutex_t ptmutex;
};

typedef struct cellpool cellpool_t;

struct variable {
    atomic_cell_ptr_t c;
    cellpool_t *pool;
    atomic_uint mutex;
    pthread_mutex_t ptmutex;
};

typedef struct variable variable_t;

#ifdef __cplusplus
extern "C" {
#endif

void set(variable_t *v, int newValue);
int get(variable_t *v);

void initPool(cellpool_t *pool, size_t nmemb);
void initVariable(variable_t *v, cellpool_t *pool);

void dumpPool(cellpool_t *pool);

#ifdef __cplusplus
}
#endif
