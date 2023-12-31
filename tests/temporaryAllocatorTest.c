#include "../sys/memory/merry_temp_alloc.h"

int main()
{
    if (merry_temp_alloc_init(1020) == RET_FAILURE)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return 0;
    }
    merry_print_allocator_info(); // get the details
    // we first test by asking for aligned memory blocks
    int *x = (int *)merry_temp_alloc(sizeof(int) * 4);
    if (x == RET_NULL)
    {
        fprintf(stderr, "Allocation failed\n");
        merry_temp_alloc_free();
        return 0;
    }
    merry_print_allocator_info(); // get the details
    merry_temp_free(x);
    merry_print_allocator_info(); // get the details
    // let us try asking for memory that is not aligned
    char *y = (char *)merry_temp_alloc(11);
    // let us allocate more blocks
    x = (int *)merry_temp_alloc(sizeof(int) * 6);
    double *z = (double *)merry_temp_alloc(sizeof(int) * 2);
    merry_print_allocator_info(); // get the details
    merry_temp_free(y);
    merry_temp_free(x);
    merry_print_allocator_info(); // get the details
    int *p = (int *)merry_temp_alloc(40);
    merry_print_allocator_info(); // get the details
    merry_temp_free(p);
    merry_temp_free(z);
    merry_print_allocator_info(); // get the details
    // lets see if the allocator correctly splits the block
    y = (char *)merry_temp_alloc(16);   // this should split the free 16 bytes
    z = (double *)merry_temp_alloc(16); // further split
    merry_print_allocator_info();       // get the details
    int *q = (int *)merry_temp_alloc(1022);
    if (q == RET_NULL)
    {
        printf("Error\n");
        return 0;
    }
    merry_print_allocator_info();       // get the details
    merry_temp_free(y);
    merry_temp_free(z);
    merry_temp_free(q);
    merry_print_allocator_info();       // get the details
    merry_temp_alloc_free();
}