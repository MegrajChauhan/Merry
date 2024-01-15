#include "../merry/lib/include/merry_memory_allocator.h"

int main()
{
    merry_allocator_init();
    char *pqr = (char *)merry_malloc(10);
    if (pqr == NULL)
    {
        printf("SHIT!\n");
    }
    else{
        *pqr = '9';
        printf("%c\n", *pqr);
    }
    merry_allocator_destroy();
}