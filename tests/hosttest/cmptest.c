#include <stdio.h>

extern void get(unsigned int x, unsigned int y, unsigned int *p);

int main()
{
    unsigned int a;
    get(0, 0, &a);
    printf("a is %u\n", a);
}