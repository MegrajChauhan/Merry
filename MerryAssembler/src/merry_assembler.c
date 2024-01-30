#include <stdio.h>

int main()
{
    FILE *f = fopen("inpFile.mbin", "wb");
    unsigned char header[24] = {0x4d, 0x49, 0x4e};
    header[7] = 0x00; // little endian
    for (int i = 8; i < 15; i++)
        header[i] = 0;
    header[15] = 32; // 16 bytes
    for (int i = 16; i < 24; i++)
        header[i] = 0;
    fwrite(header, 1, 24, f);
    unsigned char inst[32] = {};
    // first the test of add_imm
    inst[8] = 0x4E;
    inst[9] = 0x61;
    inst[10] = 0xBC; // add 12345678
    inst[11] = 0;
    inst[12] = 0;
    inst[13] = 0;
    inst[14] = 0x00; // dest = Ma
    inst[15] = 0x02;
    inst[16] = 0;
    inst[17] = 0;
    inst[18] = 0;
    inst[19] = 0;
    inst[20] = 0;
    inst[21] = 0;    // source = Ma
    inst[22] = 0x01; // dest = Mb
    inst[23] = 0x03; // add two registers
    inst[24] = 0x00;
    inst[25] = 0x00;
    inst[26] = 0x00;
    inst[27] = 0x00;
    inst[28] = 0x00;
    inst[29] = 0x00;
    inst[30] = 0x00;
    inst[31] = 0x01; // halt
    fwrite(inst, 1, 32, f);
    fclose(f);
}