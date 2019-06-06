#ifndef _YAPS_RCU_PRIVATE_H_
#define _YAPS_RCU_PRIVATE_H_

#include <stdatomic.h>

struct cell {
    int value;
    atomic_bool obsolete;
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

#endif // _YAPS_RCU_PRIVATE_H_
