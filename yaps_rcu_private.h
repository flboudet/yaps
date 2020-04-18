#ifndef _YAPS_RCU_PRIVATE_H_
#define _YAPS_RCU_PRIVATE_H_

#include <stdatomic.h>

struct cell {
    int value;
    atomic_uint rctr;
};

/**
 * Must be mapped in shared memory
 */
struct cellpool {
    //cell_ptr_t pool;
    size_t poolSize;
    atomic_size_t next;
    cell_t pool[];
};

/**
 * Must be mapped in shared memory
 */
struct variable_private {
    atomic_uint mutex;
    size_t cSize;
    atomic_size_t cur, next;
    size_t c[];
};

struct variable {
    cellpool_t *pool;
    struct variable_private *_p;
};

struct reader {
    variable_t *v;
    size_t pos;
};

#endif // _YAPS_RCU_PRIVATE_H_
