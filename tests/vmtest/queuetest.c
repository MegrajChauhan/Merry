#include <stdio.h>
#include <stdlib.h>
#include "merry_queue.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdatomic.h>

#define TEST_CAPACITY 5
#define STATIC_CAPACITY 1000    // Large capacity for static queue
#define DYNAMIC_TEST_SIZE 50000 // Huge size for dynamic queue
#define THREAD_COUNT 5          // Number of threads for concurrency test

// Thread-safe dynamic queue test
MerryDynamicQueue *global_dq;
pthread_mutex_t dq_lock;
mbool_t stop = mfalse;

// Producer function for the dynamic queue
void *dynamic_producer(void *arg)
{
    long id = (long)arg;
    for (int i = 1; i <= DYNAMIC_TEST_SIZE / THREAD_COUNT; i++)
    {
        pthread_mutex_lock(&dq_lock);
        merry_dynamic_queue_push(global_dq, (mptr_t)(long)(i + (id * 10000)));
        pthread_mutex_unlock(&dq_lock);
    }
    printf("Finished pushing\n");
    return NULL;
}

// Consumer function for the dynamic queue
void *dynamic_consumer(void *arg)
{
    mptr_t data;
    while (!stop)
    {
        pthread_mutex_lock(&dq_lock);
        if (merry_dynamic_queue_pop(global_dq, &data) == RET_SUCCESS)
        {
            printf("Consumer got: %ld\n", (long)data);
        }
        pthread_mutex_unlock(&dq_lock);
    }
    printf("Finished consuming\n");
    return NULL;
}

// Complex static queue test
void test_static_queue_limit()
{
    printf("\n--- Testing Complex MerryStaticQueue (Capacity: %d) ---\n", STATIC_CAPACITY);

    MerryStaticQueue *sq = merry_static_queue_init(STATIC_CAPACITY);
    if (!sq)
    {
        printf("Failed to initialize static queue!\n");
        return;
    }

    // **1. Wraparound Testing**
    for (int i = 1; i <= STATIC_CAPACITY; i++)
    {
        merry_static_queue_push(sq, (mptr_t)(long)i);
    }

    // Pop half the elements to test circular behavior
    mptr_t popped;
    for (int i = 0; i < STATIC_CAPACITY / 2; i++)
    {
        merry_static_queue_pop(sq, &popped);
    }

    // Push additional elements to force wraparound
    for (int i = STATIC_CAPACITY + 1; i <= STATIC_CAPACITY + (STATIC_CAPACITY / 2); i++)
    {
        merry_static_queue_push(sq, (mptr_t)(long)i);
    }

    // **2. Randomized Push/Pop Operations**
    srand(time(NULL));
    for (int i = 0; i < 10000; i++)
    {
        int op = rand() % 2; // 50% push, 50% pop
        if (op == 0)
        { // Push
            long val = rand() % 100000;
            if (merry_static_queue_push(sq, (mptr_t)(long)val) == RET_SUCCESS)
            {
                printf("Random Push: %ld\n", val);
            }
        }
        else
        { // Pop
            if (merry_static_queue_pop(sq, &popped) == RET_SUCCESS)
            {
                printf("Random Pop: %ld\n", (long)popped);
            }
        }
    }

    // **3. Final Flush**
    while (merry_static_queue_pop(sq, &popped) == RET_SUCCESS)
    {
        printf("Final Pop: %ld\n", (long)popped);
    }

    // Cleanup
    merry_static_queue_destroy(sq);
    printf("PASS: Static queue destroyed successfully.\n");
}

// Complex dynamic queue test
void test_dynamic_queue_limit()
{
    printf("\n--- Testing Complex MerryDynamicQueue ---\n");

    global_dq = merry_dynamic_queue_init();
    pthread_mutex_init(&dq_lock, NULL);
    if (!global_dq)
    {
        printf("Failed to initialize dynamic queue!\n");
        return;
    }

    // **1. Multi-threaded stress test**
    pthread_t producers[THREAD_COUNT], consumers[THREAD_COUNT];

    // Start producer threads
    for (long i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&producers[i], NULL, dynamic_producer, (void *)i);
    }

    // Start consumer threads
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&consumers[i], NULL, dynamic_consumer, NULL);
    }

    // Wait for producers
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(producers[i], NULL);
    }
    
    // Allow some consumption time before exiting
    sleep(5);
    
    // **2. Huge data test**
    for (int i = 1; i <= 100000; i++)
    {
        pthread_mutex_lock(&dq_lock);
        merry_dynamic_queue_push(global_dq, (mptr_t)(long)i);
        pthread_mutex_unlock(&dq_lock);
    }

    // mptr_t popped;
    // while (merry_dynamic_queue_pop(global_dq, &popped) == RET_SUCCESS)
    // {
    //     printf("Bulk Pop: %ld\n", (long)popped);
    // }
    
    atomic_store(&stop, mtrue);
    
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    // Cleanup
    merry_dynamic_queue_destroy(global_dq);
    pthread_mutex_destroy(&dq_lock);
    printf("PASS: Dynamic queue destroyed successfully.\n");
}

void test_dynamic_queue()
{
    printf("\n--- Testing MerryDynamicQueue ---\n");

    MerryDynamicQueue *dq = merry_dynamic_queue_init();
    if (!dq)
    {
        printf("Failed to initialize dynamic queue!\n");
        return;
    }

    mptr_t popped_data;
    if (merry_dynamic_queue_pop(dq, &popped_data) == RET_FAILURE)
        printf("PASS: Pop from empty dynamic queue failed as expected.\n");

    // Pushing values
    for (int i = 1; i <= 10; i++)
    {
        if (merry_dynamic_queue_push(dq, (mptr_t)(long)i) == RET_SUCCESS)
            printf("Pushed %d to dynamic queue.\n", i);
        else
            printf("FAILED: Could not push %d to dynamic queue.\n", i);
    }

    // Edge: Pushing NULL[Passed]
    // if (merry_dynamic_queue_push(dq, NULL) == RET_FAILURE)
    //     printf("PASS: Cannot push NULL to dynamic queue.\n");

    // Popping all values
    while (merry_dynamic_queue_pop(dq, &popped_data) == RET_SUCCESS)
        printf("Popped %ld from dynamic queue.\n", (long)popped_data);

    // Edge: Clearing after operations
    merry_dynamic_queue_clear(dq);
    if (dq->data_count == 0)
        printf("PASS: Dynamic queue cleared successfully.\n");

    // Destroy queue
    merry_dynamic_queue_destroy(dq);
    printf("PASS: Dynamic queue destroyed successfully.\n");
}

void test_static_queue()
{
    printf("\n--- Testing MerryStaticQueue (Capacity: %d) ---\n", TEST_CAPACITY);

    MerryStaticQueue *sq = merry_static_queue_init(TEST_CAPACITY);
    if (!sq)
    {
        printf("Failed to initialize static queue!\n");
        return;
    }

    // Edge: Popping from empty queue
    mptr_t popped_data;
    if (merry_static_queue_pop(sq, &popped_data) == RET_FAILURE)
        printf("PASS: Pop from empty static queue failed as expected.\n");

    // Pushing up to capacity
    for (int i = 1; i <= TEST_CAPACITY; i++)
    {
        if (merry_static_queue_push(sq, (mptr_t)(long)i) == RET_SUCCESS)
            printf("Pushed %d to static queue.\n", i);
        else
            printf("FAILED: Could not push %d to static queue.\n", i);
    }

    // Edge: Pushing beyond capacity
    if (merry_static_queue_push(sq, (mptr_t)(long)999) == RET_FAILURE)
        printf("PASS: Static queue rejected push beyond capacity.\n");

    // Popping all values
    while (merry_static_queue_pop(sq, &popped_data) == RET_SUCCESS)
        printf("Popped %ld from static queue.\n", (long)popped_data);

    // Edge: Clearing after operations
    merry_static_queue_clear(sq);
    if (sq->data_count == 0)
        printf("PASS: Static queue cleared successfully.\n");

    // Destroy queue
    merry_static_queue_destroy(sq);
    printf("PASS: Static queue destroyed successfully.\n");
}

int main()
{
    test_dynamic_queue();
    test_static_queue();
    test_dynamic_queue_limit();
    test_static_queue_limit();
    return 0;
}
