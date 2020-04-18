#include <stdio.h>
#include <thread>
#include <mutex>
#include <gtest/gtest.h>
#include "yaps_rcu.h"

TEST(HistoryTest, ReadSingleVariable)
{
    variable_t *toto;
    cellpool_t *totoPool;
    totoPool = allocInitPool(80);
    toto = allocInitVariable(totoPool, 20);
    ::set(toto, 0);
    reader_t *totoReader = allocInitReader(toto);
    for (int i = 1 ; i < 10 ; ++i) {
        ::set(toto, i);
    }
    for (int i = 0 ; i < 10 ; ++i) {
        std::cout << reader_get(totoReader) << std::endl;
    }
}
