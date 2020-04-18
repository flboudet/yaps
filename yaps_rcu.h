#ifndef _YAPS_RCU_H_
#define _YAPS_RCU_H_

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
using namespace std;
# define _Atomic(X) std::atomic< X >
#endif

#include <stddef.h>

typedef struct cell cell_t;
typedef cell_t * cell_ptr_t;
typedef cell_ptr_t atomic_cell_ptr_t;

struct cellpool;

typedef struct cellpool cellpool_t;

struct variable;
typedef struct variable variable_t;

struct reader;
typedef struct reader reader_t;

#ifdef __cplusplus
extern "C" {
#endif

void set(variable_t *v, int newValue);
int get(variable_t *v);

// Pool API
void initPool(cellpool_t *pool, size_t nmemb);
cellpool_t *allocInitPool(size_t nmemb);
size_t pool_getMemSizeOf(cellpool_t *pool);
void dumpPool(cellpool_t *pool);

// Variable API
/**
 * \brief Initialize a variable.
 */
void initVariable(variable_t *v, size_t nmemb);
/**
 * \brief Create a variable_t pointing at a known variable memory.
 * The variable is not initialized by this function.
 */
variable_t *mapVariable(cellpool_t *pool, void *vmemory);
/**
 * \brief Create a newly allocated and initialized a variable
 */
variable_t *allocInitVariable(cellpool_t *pool, size_t depth);

// Reader API
reader_t *allocInitReader(variable_t *v);
int reader_get(reader_t *r);

#ifdef __cplusplus
}
#endif

#endif // _YAPS_RCU_H_
