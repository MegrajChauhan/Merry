#include <stdio.h>

int main()
{
    FILE *f = fopen("order.txt", "wb");
    unsigned char byte[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01, 0x02, 0x03, 0x07,0x08, 0x09,0x10, 0x11, 0x12, 0x13};
    fwrite(byte, 1, 16, f);
    unsigned long long x[2];
    fclose(f);
    f = fopen("order.txt", "rb");
    fread(&x, 8, 2, f);
    printf("READ: %llx %llx\n", x[0], x[1]);
    fclose(f);
    char *y = (char*)x;
    f = fopen("order.txt", "wb");
    fwrite(y, 1, 16, f);
    fclose(f);
    f = fopen("order.txt", "rb");
    fread(byte, 1, 16, f);
    for (int i = 0; i < 16; i ++)
    {
        printf("%X\n", byte[i]);
    }
    fclose(f);
}