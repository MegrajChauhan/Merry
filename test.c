#include <stdio.h>
#include <stdlib.h>

int main() {
    const int num_allocations = 100;
    const size_t allocation_size = 1 * 1024 * 1024; // 1 MB
    void* pointers[num_allocations];

    for (int i = 0; i < num_allocations; ++i) {
        pointers[i] = malloc(allocation_size);
        if (pointers[i] == NULL) {
            printf("Allocation failed at iteration %d\n", i);
            break;
        } else {
            printf("Allocated 1 MB at iteration %d\n", i);
        }
    }

    getchar();

    // Free the allocated memory
    for (int i = 0; i < num_allocations; ++i) {
        if (pointers[i] != NULL) {
            free(pointers[i]);
        }
    }

    return 0;
}
