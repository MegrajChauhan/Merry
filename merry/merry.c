#include "../utils/merry_config.h"
#include "../utils/merry_types.h"
#include "../sys/merry_thread.h"
#include "lib/include/merry_memory_allocator.h"
#include <stdio.h>

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
}