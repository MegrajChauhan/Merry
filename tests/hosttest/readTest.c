#include <stdio.h>

void write(char *x)
{
    *x = 'h';
}

// extern void read(unsigned char *p, unsigned long *x);
int main()
{
    // unsigned char p[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    // unsigned long A;
    // unsigned long *x = &A;
    // read(p, x);
    // printf("x is %lX\n", *x);
    unsigned long a = 0;
    write(&a);
    printf("%lu\n", a);
}