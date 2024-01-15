#include <stdio.h>

int main()
{
    FILE *f = fopen("binfile.mbin", "rb");
    if (f == NULL)
    {
        printf("Failed to open file.\n");
        return 0;
    }
    unsigned char buffer[12];
    fread(buffer, 1, 5, f);
    for (int i = 0; i < 5; i++)
    {
        printf("%X\n", buffer[i]);
    }
    // buffer[0] = 0x67;
    // buffer[1] = 0x45;
    // buffer[2] = 0xAB;
    // buffer[3] = 0xCD;
    // fwrite(buffer, 1, 4, f);
    // printf("Written\n");
    // fread(buffer, 1, 4, f);
    // for (int i = 0; i < 4; i++)
    // {
    //     printf("%X\n", buffer[i]);
    // }
    fseek(f, SEEK_SET, SEEK_END);
    printf("SIZE: %lu\n", ftell(f));
    rewind(f);
    unsigned int* x = buffer;
    printf("READ: %X\n",*x);
    fclose(f);
}