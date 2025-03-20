// we cannot test dynlib yet

#include <stdio.h>
#include <merry_list.h>
#include <stdio.h>
#include "merry_list.h"

void test_create_list() {
    printf("Testing list creation... ");

    MerryList *list = merry_create_list(10, sizeof(int));
    if (!list || !list->buf || list->buf_cap != 10 || list->elem_len != sizeof(int)) {
        printf("FAILED\n");
        exit(EXIT_FAILURE);
    }

    printf("PASSED\n");
    merry_destroy_list(list);
}

void test_push_and_pop() {
    printf("Testing push and pop operations... ");

    MerryList *list = merry_create_list(5, sizeof(int));
    int values[] = {1, 2, 3, 4, 5};

    for (int i = 0; i < 5; i++) {
        if (merry_list_push(list, &values[i]) != RET_SUCCESS) {
            printf("FAILED on push\n");
            exit(EXIT_FAILURE);
        }
    }

    if (merry_list_push(list, &values[0]) == RET_SUCCESS) {  // Should fail (list full)
        printf("FAILED: Pushed beyond capacity\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 4; i >= 0; i--) {
        int *popped = (int *)merry_list_pop(list);
        if (!popped || *popped != values[i]) {
            printf("FAILED on pop (expected %d, got %d)\n", values[i], popped ? *popped : -1);
            exit(EXIT_FAILURE);
        }
    }

    if (merry_list_pop(list) != RET_NULL) {  // Should return NULL (list empty)
        printf("FAILED: Popped from empty list\n");
        exit(EXIT_FAILURE);
    }

    printf("PASSED\n");
    merry_destroy_list(list);
}

void test_resize() {
    printf("Testing list resize... ");

    MerryList *list = merry_create_list(3, sizeof(int));
    int values[] = {10, 20, 30};

    for (int i = 0; i < 3; i++) {
        merry_list_push(list, &values[i]);
    }

    if (merry_list_resize_list(list, 2) != RET_SUCCESS) {  // Resize from 3 to 6
        printf("FAILED: Resize failed\n");
        exit(EXIT_FAILURE);
    }

    if (list->buf_cap != 6) {
        printf("FAILED: Resize did not update capacity correctly (expected 6, got %ld)\n", list->buf_cap);
        exit(EXIT_FAILURE);
    }

    printf("PASSED\n");
    merry_destroy_list(list);
}

void test_add_capacity() {
    printf("Testing adding capacity... ");

    MerryList *list = merry_create_list(4, sizeof(int));
    int values[] = {1, 2, 3, 4};

    for (int i = 0; i < 4; i++) {
        merry_list_push(list, &values[i]);
    }

    if (merry_add_capacity_to_list(list, 3) != RET_SUCCESS) {  // Increase from 4 to 7
        printf("FAILED: Could not add capacity\n");
        exit(EXIT_FAILURE);
    }

    if (list->buf_cap != 7) {
        printf("FAILED: Capacity not updated correctly (expected 7, got %ld)\n", list->buf_cap);
        exit(EXIT_FAILURE);
    }

    printf("PASSED\n");
    merry_destroy_list(list);
}

void test_clear() {
    printf("Testing list clear... ");

    MerryList *list = merry_create_list(5, sizeof(int));
    int value = 42;

    merry_list_push(list, &value);
    merry_list_push(list, &value);

    merry_erase_list(list);

    if (!merry_is_list_empty(list)) {
        printf("FAILED: List not cleared properly\n");
        exit(EXIT_FAILURE);
    }

    printf("PASSED\n");
    merry_destroy_list(list);
}

int main() {
    test_create_list();
    test_push_and_pop();
    test_resize();
    test_add_capacity();
    test_clear();

    printf("All tests PASSED!\n");
    return 0;
}


// let me test this first
// so i wrote a series of tests actually and i found a slight bug