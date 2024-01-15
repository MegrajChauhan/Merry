#include <stdio.h>

int main()
{
    FILE *f = fopen("test.mbin", "wb");
    if (f == NULL)
    {
        printf("Failed to open file.\n");
        return 0;
    }
    unsigned char wr[8] = {0x4d, 0x49, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00};
    fwrite(wr, 1, 8, f);
    fclose(f);
}