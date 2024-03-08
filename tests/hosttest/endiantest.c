#include <stdio.h>

int main()
{
    unsigned char test1[16] = {0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15};
    unsigned char *x = &test1;
    unsigned long long *y = (unsigned long long *)(x);
    unsigned long long z = *y;
    printf("PTR x: %p\nPTR y:%p\n", x, y);
    printf("At x: %u\nAt y:%lX\nAt z: %lX\n", *x, *y, z);
    // x++;
    // y++;
    // printf("PTR x: %p\nPTR y:%p\n", x, y);
    // printf("At x: %u\nAt y:%X\n", *x, *y);
    // x += 7;
    // printf("PTR x: %p\nPTR y:%p\n", x, y);
    if ((*y >> 56) == 0x7)
    {
        printf("Correct!\n");
    }
    if ((z >> 56) == 0x7)
    {
        printf("Correct Again!\n");
    }
    unsigned long long p = 0xFFEEDDCCBBAA1122;
    *y = p;
}