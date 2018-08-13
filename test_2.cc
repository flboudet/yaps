#include <stdio.h>
#include <thread>
#include "yaps_rcu.h"

void iterateOnVariable(variable_t *v)
{
    for (int i = 0 ; i < 1000 ; ++i)
        set(v, i);
}

int main()
{
    variable_t toto, titi;
    cellpool_t totoPool;
    initPool(&totoPool, 128);
    initVariable(&toto, &totoPool);
    initVariable(&titi, &totoPool);

    set(&toto, 6666);
    set(&titi, 6666);

    std::thread iterateOnToto(iterateOnVariable, &toto);
    std::thread iterateOnTiti(iterateOnVariable, &titi);
    //std::this_thread::sleep_for(2s);

    //printf("get toto: %d\n", get(&toto));
    //printf("get titi: %d\n", get(&titi));

    iterateOnToto.join();
    iterateOnTiti.join();

    printf("get toto: %d\n", get(&toto));
    printf("get titi: %d\n", get(&titi));
    return 0;
}
