#include <stdio.h>
#include <string.h>

int main()
{
    volatile char x[1024 * 1024]; // copying this really only took only a few miliseconds
    volatile char y[1024 * 1024];
    memcpy(y, x, 1024 * 1024);
}