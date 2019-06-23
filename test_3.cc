#include <sys/mman.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include "yaps_rcu.h"


void parentProcess(void *shared_mem)
{
    char *shared_mem_pos = (char *)shared_mem;
    cellpool_t *totoPool = (cellpool_t *)shared_mem;
    initPool(totoPool, 80);
    shared_mem_pos += pool_getMemSizeOf(totoPool);
    variable_private *totoVp = (variable_private *)shared_mem_pos;
    variable_t *toto = mapVariable(totoPool, totoVp, 1);
    initVariable(totoVp, totoPool, 1);
    ::set(toto, 31415);
}

void childProcess(void *shared_mem)
{
    char *shared_mem_pos = (char *)shared_mem;
    cellpool_t *totoPool = (cellpool_t *)shared_mem_pos;
    shared_mem_pos += pool_getMemSizeOf(totoPool);
    variable_private *totoVp = (variable_private *)shared_mem_pos;
    variable_t *toto = mapVariable(totoPool, totoVp, 1);

    sleep(1);
    printf("Bla bla\n");
    printf("toto val: %d\n", get(toto));
    ASSERT_EQ(31415, get(toto));
}

TEST(MultiprocessTest, SingleVariable)
{
    size_t size = 4096*100;
    void *shared_mem = mmap(NULL, size,
                            PROT_READ | PROT_WRITE,
                            MAP_ANONYMOUS | MAP_SHARED,
                            -1, 0);
    //shared_mem = malloc(size);
    int pid = fork();
    if (pid == 0) {
        printf("Child process\n");
        childProcess(shared_mem);
        exit(0);
    }
    else {
        printf("Parent process\n");
        parentProcess(shared_mem);
        int wstatus;
        waitpid(pid, &wstatus, 0);
    }
}
