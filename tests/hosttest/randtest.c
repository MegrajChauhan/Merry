#include <stdio.h>
#include <stdlib.h>

#define r (unsigned)rand()

int main()
{
    srand(123);
    int x[12] = {
        r, r, r, r, r, r, r, r, r, r, r, r};
    for (__ssize_t i = 0; i < 12; i++)
    {
        printf("%d\n", x[i]);
    }
}