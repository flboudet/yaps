#include <sys/time.h>
#include "yaps_rcu.h"
#include <gtest/gtest.h>

#define NB_ITER 4e6

uint64_t getTimestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t result = tv.tv_sec * 1e6 + tv.tv_usec;
    return result;
}

TEST(PerfTest, SetSingleVariable)
{
    variable_t *toto;
    cellpool_t *totoPool;
    totoPool = allocInitPool(80);
    toto = allocInitVariable(totoPool, 1);
    uint64_t initialTime = getTimestamp();
    for (int i = 0 ; i < NB_ITER ; ++i) {
        ::set(toto, i);
    }
    uint64_t finalTime = getTimestamp();
    ASSERT_EQ(NB_ITER - 1, get(toto));
    printf("Time per set: %f us\n", (double)(finalTime - initialTime)/NB_ITER);
}

TEST(PerfTest, GetSingleVariable)
{
    variable_t *toto;
    cellpool_t *totoPool;
    totoPool = allocInitPool(80);
    toto = allocInitVariable(totoPool, 1);
    ::set(toto, 1);
    uint64_t initialTime = getTimestamp();
    for (int i = 0 ; i < NB_ITER ; ++i) {
        ::get(toto);
    }
    uint64_t finalTime = getTimestamp();
    ASSERT_EQ(1, get(toto));
    printf("Time per get: %f us\n", (double)(finalTime - initialTime)/NB_ITER);
}
