// #define _MERRY_LOGGER_ENABLED_

#include "internals/merry_os.h"
#include <stdio.h>
#include <time.h>

// a thread func
// mptr_t test_func(mptr_t val)
// {
//   // val is 0
//   int x = *(int *)val;
//   while (x < 100)
//   {
//     x++;
//   }
//   return NULL;
// }
// printf("Hello World from Merry!\n");
// // some testing
// if (merry_allocator_init() == RET_FAILURE)
// {
//   printf("Allocator init failed.\n");
//   return 0;
// }
// print_details();
// MerryThread *th = merry_thread_init();
// print_details();
// if (th == NULL)
// {
//   printf("Thread init failed.\n");
// }
// int p = 0;
// if (merry_create_thread(th, &test_func, &p) == RET_FAILURE)
// {
//   printf("Thread initialization failed.\n");
// }
// else
// {
//   merry_thread_join(th, NULL);
// }
// getc(stdin);
// print_details();
// merry_thread_destroy(th);
// print_details();
// merry_allocator_destroy();

int main()
{
    merry_init_logger();
    if (merry_os_init("inpFile.mbin") == RET_FAILURE)
        return 0;
    // Merry *temp; // temporary
    // get(&temp);
    MerryThread *osthread = merry_thread_init();
    if (osthread == NULL)
    {
        fprintf(stderr, "Failed to intialize VM.\n");
        goto failure;
    }
    clock_t start = clock();
    if (merry_create_thread(osthread, &merry_os_start_vm, NULL) == RET_FAILURE)
    {
        fprintf(stderr, "Failed to start VM.\n");
        merry_thread_destroy(osthread);
        goto failure;
    }
    merry_close_logger();
    merry_thread_join(osthread, NULL); // I am an idiot
    clock_t end = clock();
    merry_thread_destroy(osthread);
    merry_os_destroy();
    printf("Time taken to run: %lfs.\n", (double)(end - start) / CLOCKS_PER_SEC);
    return 0;
failure:
    merry_close_logger();
    merry_os_destroy();
    return -1;
}

/*
 No optimizations:
 1 million loops: 0.162401s
 1 Billion loops: 140.932465s(Holy crap!)[2 minutes and 20 seconds! Jesus!]

 Optimizations(-03):
 1 Million loops:
 1 Billion loops: 84.5s
*/