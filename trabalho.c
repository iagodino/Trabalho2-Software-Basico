#include <stdio.h>
#include "mymemory.h"

int main()
{
    // aloca pool memoria
    mymemory_t *memory = mymemory_init(1000);

    //int v[32];
    // int *v = malloc(sizeof(int)*32);
    int *v = mymemory_malloc(memory, sizeof(int)*32);
    for (int i = 0; i < 32; i++)
        v[i] = i+1;


    int *v2 = mymemory_malloc(memory, sizeof(int)*16);

    // free(v);
    mymemory_free(memory, v);

    char *str = mymemory_malloc(memory, sizeof(char)*16);
    sprintf(str, "ola mundo");

    // libera o pool de memoria
    mymemory_release(memory);

    return 0;
}
