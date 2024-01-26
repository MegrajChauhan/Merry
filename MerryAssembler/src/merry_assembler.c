#include <stdio.h>

int main()
{
    FILE *f = fopen("inpFile.mbin", "wb");
    unsigned char header[24] = {0x4d, 0x49, 0x4e};
    header[7] = 0x00; // little endian
    for (int i = 8; i < 15; i++)
        header[i] = 0;
    header[15] = 0x10; // 16 bytes
    for (int i = 16; i < 24; i++)
        header[i] = 0;
    fwrite(header, 1, 24, f);
    unsigned char inst[16] = {};
    inst[8] = 0x01;
    fwrite(inst, 1, 16, f);
    fclose(f);
}