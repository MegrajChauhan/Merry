#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "merry_stack.h"

#define TEST_CAPACITY 5
#define NUM_THREADS 4
#define NUM_OPERATIONS 10000

void test_basic_stack()
{
    printf("Running basic stack tests...\n");

    MerryStack *stack = merry_stack_init(TEST_CAPACITY);
    if (!stack)
    {
        printf("Failed to initialize stack!\n");
        return;
    }

    for (int i = 1; i < TEST_CAPACITY; i++)
    {
        if (merry_stack_push(stack, (mptr_t)(long)i) != RET_SUCCESS)
        {
            printf("Push failed unexpectedly at %d\n", i);
        }
    }

    if (merry_stack_push(stack, (mptr_t)(long)100) == RET_SUCCESS)
    {
        printf("ERROR: Stack allowed push beyond capacity!\n");
    }

    for (int i = TEST_CAPACITY - 1; i > 0; i--)
    {
        mptr_t val = merry_stack_pop(stack);
        if ((long)val != i)
        {
            printf("ERROR: Stack pop order incorrect! Expected %d, got %ld\n", i, (long)val);
        }
        printf("VALUE: %ld\n", (long)val);
    }

    if (merry_stack_pop(stack) != NULL)
    {
        printf("ERROR: Stack pop on empty stack should return NULL!\n");
    }

    merry_stack_clear(stack);
    if (!merry_is_stack_empty(stack))
    {
        printf("ERROR: Stack clear did not reset stack!\n");
    }

    merry_stack_destroy(stack);
    printf("Basic stack tests completed.\n");
}

int main()
{
    test_basic_stack();
    return 0;
}
