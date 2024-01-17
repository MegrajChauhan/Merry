#ifndef _MERRY_QUEUE_
#define _MERRY_QUEUE_

#include "../../../utils/merry_types.h"
#include "../../../utils/merry_config.h"
#include "../../../utils/merry_utils.h"
#include "merry_memory_allocator.h"

#define _MERRY_CREATE_QUEUE_NODE_(type, name) \
    struct name                               \
    {                                         \
        type *value;                          \
        struct name *next;                    \
    };

// suffix could be anything but for the VM it would be something like CharQueue, IntQueue
#define _MERRY_CREATE_QUEUE_(name, node_name)                                                                              \
    struct name                                                                                                            \
    {                                                                                                                      \
        msize_t node_count;     /* The queue is of fixed size.Once initialized with a size, the size cannot be changed. */ \
        msize_t data_count;     /* The number of data pushed to the queue so far*/                                         \
        struct node_name *head; /*The head of the queue's linked list.*/                                                   \
        struct node_name *tail; /*The tail of the queue's linked list*/                                                    \
    };

// is queue empty?
#define _MERRY_IS_QUEUE_EMPTY_(qptr) (qptr->data_count == 0)
// is the queue full?
#define _MERRY_IS_QUEUE_FULL_(qptr) (qptr->data_count == qptr->node_count)

#define _MERRY_QUEUE_INIT_(queue_name, qptr)                   \
    do                                                         \
    {                                                          \
        qptr = (queue_name *)merry_malloc(sizeof(queue_name)); \
    } while (0);
// checking if the pointer was initialized is the caller's job

#define _MERRY_QUEUE_CREATE_NODES_(qptr, node_name, create_node_count, ret)                        \
    do                                                                                             \
    {                                                                                              \
        qptr->node_count = (create_node_count);                                                    \
        qptr->head = (node_name *)merry_malloc(sizeof(node_name));                                 \
        if (qptr->head == NULL)                                                                    \
        {                                                                                          \
            ret = mfalse;                                                                          \
            break;                                                                                 \
        }                                                                                          \
        node_name *current = (qptr)->head;                                                         \
        for (msize_t i = 0; i < (create_node_count); i++)                                          \
        {                                                                                          \
            current->value = (typeof(current->value))merry_malloc(sizeof(typeof(current->value))); \
            if (current->value == NULL)                                                            \
            {                                                                                      \
                (ret) = mfalse;                                                                    \
                break;                                                                             \
            }                                                                                      \
            current->next = (node_name *)merry_malloc(sizeof(node_name));                          \
            if (current->next == NULL)                                                             \
            {                                                                                      \
                (ret) = mfalse;                                                                    \
                break;                                                                             \
            }                                                                                      \
            current = current->next;                                                               \
        }                                                                                          \
        current->next = (qptr)->head;                                                              \
        (qptr->tail) = (qptr)->head; /*The queue is empty*/                                        \
        ret = mtrue;                                                                               \
    } while (0);

// push a value to the queue
#define _MERRY_QUEUE_PUSH_(qptr, pvalue, ret) \
    do                                        \
    {                                         \
        if (_MERRY_IS_QUEUE_FULL_(qptr))      \
        {                                     \
            ret = mfalse;                     \
            break;                            \
        }                                     \
        *((qptr)->tail->value) = (pvalue);    \
        (qptr)->data_count++;                 \
        (qptr)->tail = (qptr)->tail->next;    \
        ret = mtrue;                          \
    } while (0);

// pop a value from the queue
// dest is not a pointer
#define _MERRY_QUEUE_POP_(qptr, dest, ret) \
    do                                     \
    {                                      \
        if (_MERRY_IS_QUEUE_EMPTY_(qptr))  \
        {                                  \
            ret = mfalse;                  \
            break;                         \
        }                                  \
        dest = *((qptr)->head->value);     \
        (qptr)->head = (qptr)->head->next; \
        (qptr)->data_count--;              \
        ret = mtrue;                       \
    } while (0);

// free the queue
#define _MERRY_DESTROY_QUEUE_(qptr)                      \
    do                                                   \
    {                                                    \
        if (qptr == NULL)                                \
            break;                                       \
        if (qptr->head == NULL)                          \
            break;                                       \
        typeof(*(qptr)->head) *current = (qptr)->head;   \
        typeof(*(qptr)->head) *next;                     \
        for (msize_t i = 0; i < (qptr)->node_count; i++) \
        {                                                \
            next = current->next;                        \
            if (current != NULL)                         \
            {                                            \
                if (current->value != NULL)              \
                    merry_free(current->value);          \
                merry_free(current);                     \
            }                                            \
            current = next;                              \
        }                                                \
        (qptr)->head = NULL;                             \
        (qptr)->tail = NULL;                             \
        (qptr)->node_count = 0;                          \
        merry_free(qptr);                                \
    } while (0);

#endif