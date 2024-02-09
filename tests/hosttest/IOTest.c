#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *_print_thread_(void *arg)
{
    for (int i = 0; i < 99; i++)
    {
        printf("Thread 1: printing\n");
        sleep(1);
    }
}

void *_read_thread_(void *arg)
{
    int x = 0;
    for (int i = 0; i < 99; i++)
    {
        printf("Thread 2: reading: ");
        scanf("%d", &x);
        sleep(1);
    }
}

int main()
{
    pthread_t threads[2];
    // threads[0] = pthread_create(threads[0], )
    pthread_create(&threads[0], NULL, &_print_thread_, NULL);
    pthread_create(&threads[1], NULL, &_read_thread_, NULL);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
}