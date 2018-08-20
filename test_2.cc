#include <stdio.h>
#include <thread>
#include "yaps_rcu.h"

void iterateOnVariable(variable_t *v, int start, int nbIter)
{
    for (int i = start ; i < nbIter ; ++i)
        set(v, i);
}

int main()
{
    variable_t toto, titi;
    cellpool_t totoPool;
    initPool(&totoPool, 8);
    initVariable(&toto, &totoPool);
    initVariable(&titi, &totoPool);

    set(&toto, 6666);
    set(&titi, 6666);

    //iterateOnVariable(&toto, 0, 1000);
    std::thread iterateOnToto(iterateOnVariable, &toto, 0, 1000);
    std::thread iterateOnTiti(iterateOnVariable, &titi, 2000, 10000);
    //std::this_thread::sleep_for(2s);

    //printf("get toto: %d\n", get(&toto));
    //printf("get titi: %d\n", get(&titi));

    iterateOnToto.join();
    iterateOnTiti.join();

    printf("get toto: %d\n", get(&toto));
    printf("get titi: %d\n", get(&titi));
    return 0;
}
