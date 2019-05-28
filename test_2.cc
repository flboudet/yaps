#include <stdio.h>
#include <thread>
#include <mutex>
#include <gtest/gtest.h>
#include "yaps_rcu.h"

std::mutex bkl;

//#define CHEAT_LOCK

void iterateOnVariable(variable_t *v, int start, int nbIter)
{
    for (int i = start ; i < nbIter ; ++i) {
#ifdef CHEAT_LOCK
        std::lock_guard<std::mutex> lock(bkl);
#endif
        ::set(v, i);
        //printf("val: %d\n", i);
    }
}

void watchVariable(variable_t *v, int start, int max) {
    int wval = start, pwval = 0;
    do {
        {
#ifdef CHEAT_LOCK
            std::lock_guard<std::mutex> lock(bkl);
#endif
            wval = ::get(v);
        }
        //printf("wval: %d\n", wval);
        ASSERT_LE(pwval,  wval);
        pwval = wval;
    } while (wval != max);
}


TEST(MultithreadTest, SingleVariable)
{
    variable_t toto;
    cellpool_t totoPool;
    initPool(&totoPool, 80);
    initVariable(&toto, &totoPool);

    ::set(&toto, 0);

    std::thread iterateOnToto(iterateOnVariable, &toto, 0, 1000);
    std::thread watchToto(watchVariable, &toto, 0, 999);
    iterateOnToto.join();
    watchToto.join();

    ASSERT_EQ(999, get(&toto));
}

TEST(MultithreadTest, TwoVariables)
{
    variable_t toto, titi;
    cellpool_t totoPool;
    cellpool_t totoPool2;
    initPool(&totoPool, 80);
    initPool(&totoPool2, 80);
    initVariable(&toto, &totoPool);
    initVariable(&titi, &totoPool);

    ::set(&toto, 0);
    ::set(&titi, 666);

    //iterateOnVariable(&toto, 0, 1000);
    std::thread iterateOnToto(iterateOnVariable, &toto, 0, 1000);
    std::thread iterateOnTiti(iterateOnVariable, &titi, 2000, 10000);
    std::thread watchToto(watchVariable, &toto, 0, 999);
    std::thread watchTiti(watchVariable, &titi, 2000, 9999);

    iterateOnToto.join();
    iterateOnTiti.join();
    watchToto.join();
    watchTiti.join();

    ASSERT_EQ(999, get(&toto));
    ASSERT_EQ(9999, get(&titi));
}

TEST(PoolAlloc, Toto)
{
    cellpool_t totoPool;
    variable_t toto, titi;

    std::cout << "*** initPool" << std::endl;
    initPool(&totoPool, 20);
    dumpPool(&totoPool);

    std::cout << "*** initVariable(toto)" << std::endl;
    initVariable(&toto, &totoPool);
    dumpPool(&totoPool);

    std::cout << "*** initVariable(titi)" << std::endl;
    initVariable(&titi, &totoPool);
    dumpPool(&totoPool);

    std::cout << "*** set(toto)" << std::endl;
    ::set(&toto, 1);
    dumpPool(&totoPool);

    std::cout << "*** set(titi)" << std::endl;
    ::set(&titi, 2);
    dumpPool(&totoPool);

    ASSERT_EQ(1, ::get(&toto));
    ASSERT_EQ(2, ::get(&titi));
    dumpPool(&totoPool);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
