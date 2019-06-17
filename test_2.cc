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
    variable_t *toto;
    cellpool_t *totoPool;
    totoPool = allocInitPool(80);
    toto = allocInitVariable(totoPool, 1);

    ::set(toto, 0);

    std::thread iterateOnToto(iterateOnVariable, toto, 0, 1000);
    std::thread watchToto(watchVariable, toto, 0, 999);
    iterateOnToto.join();
    watchToto.join();

    ASSERT_EQ(999, get(toto));
}

TEST(MultithreadTest, TwoVariables)
{
    variable_t *toto, *titi;
    cellpool_t *totoPool;
    cellpool_t *totoPool2;
    totoPool = allocInitPool(80);
    totoPool2 = allocInitPool(80);
    toto = allocInitVariable(totoPool, 1);
    titi = allocInitVariable(totoPool, 1);

    ::set(toto, 0);
    ::set(titi, 666);

    //iterateOnVariable(toto, 0, 1000);
    std::thread iterateOnToto(iterateOnVariable, toto, 0, 1000);
    std::thread iterateOnTiti(iterateOnVariable, titi, 2000, 10000);
    std::thread watchToto(watchVariable, toto, 0, 999);
    std::thread watchTiti(watchVariable, titi, 2000, 9999);

    iterateOnToto.join();
    iterateOnTiti.join();
    watchToto.join();
    watchTiti.join();

    ASSERT_EQ(999, get(toto));
    ASSERT_EQ(9999, get(titi));
}

TEST(MultithreadTest, NVariablesOnePool)
{
    int nbV = 100;
    variable_t *variables[nbV];
    cellpool_t *uniquePool;
    uniquePool = allocInitPool(nbV*10);
    for (int i = 0 ; i < nbV ; ++i) {
        variables[i] = allocInitVariable(uniquePool, 1);
        ::set(variables[i], 0);
    }

    std::vector<std::thread> iterateThreads;
    std::vector<std::thread> watchThreads;
    for (int i = 0 ; i < nbV ; ++i) {
        int min = i*100;
        int max = min + 1000;
        iterateThreads.push_back(std::thread(iterateOnVariable, variables[i], min, max));
        watchThreads.push_back(std::thread(watchVariable, variables[i], min, max - 1));
    }
    for (std::thread &t : iterateThreads)
        t.join();
    for (std::thread &t : watchThreads)
        t.join();

    for (int i = 0 ; i < nbV ; ++i) {
        int min = i*100;
        int max = min + 1000;
        ASSERT_EQ(max - 1, get(variables[i]));
    }
}

TEST(PoolAlloc, Toto)
{
    cellpool_t *totoPool;
    variable_t *toto, *titi;

    std::cout << "*** allocInitPool" << std::endl;
    totoPool = allocInitPool(20);
    dumpPool(totoPool);

    std::cout << "*** initVariable(toto)" << std::endl;
    toto = allocInitVariable(totoPool, 1);
    dumpPool(totoPool);

    std::cout << "*** initVariable(titi)" << std::endl;
    titi = allocInitVariable(totoPool, 1);
    dumpPool(totoPool);

    std::cout << "*** set(toto)" << std::endl;
    ::set(toto, 1);
    dumpPool(totoPool);

    std::cout << "*** set(titi)" << std::endl;
    ::set(titi, 2);
    dumpPool(totoPool);

    ASSERT_EQ(1, ::get(toto));
    ASSERT_EQ(2, ::get(titi));
    dumpPool(totoPool);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
