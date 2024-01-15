#include "internals/merry_reader.h"

int main()
{
    MerryInpFile *inp = merry_read_file("test.mbin");
    if (inp == RET_NULL)
    {
        printf("Failure\n");
        return 0;
    }
    merry_destory_reader(inp);
}