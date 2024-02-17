#include <stdio.h>

unsigned int print(void *ptr)
{
    unsigned long x = *(unsigned long *)ptr;
    printf("Call says %lu\n", x);
}