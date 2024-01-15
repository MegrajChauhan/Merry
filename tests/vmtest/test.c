#include "../merry/internals/merry_reader.h"

int main()
{
    MerryInpFile *inp = merry_read_file("testfile.mbin");
    if (inp == RET_NULL)
    {
        printf("Failure\n");
    }
    merry_destory_reader(inp);
}