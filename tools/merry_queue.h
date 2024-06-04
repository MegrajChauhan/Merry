/*
 * A blueprint of a queue of the Merry VM
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _MERRY_QUEUE_
#define _MERRY_QUEUE_

#include <stdlib.h>
#include <merry_utils.h>
#include <merry_types.h>

#define _MERRY_CREATE_QUEUE_NODE_(type, name) \
    struct name                               \
    {                                         \
        type *value;                          \
        struct name *next;                    \
    }

// suffix could be anything but for the VM it would be something like CharQueue, IntQueue
#define _MERRY_CREATE_QUEUE_(name, node_name)                                                                              \
    struct name                                                                                                            \
    {                                                                                                                      \
        msize_t node_count;     /* The queue is of fixed size.Once initialized with a size, the size cannot be changed. */ \
        msize_t data_count;     /* The number of data pushed to the queue so far*/                                         \
        struct node_name *head; /*The head of the queue's linked list.*/                                                   \
        struct node_name *tail; /*The tail of the queue's linked list*/                                                    \
    }

// is queue empty?
#define _MERRY_IS_QUEUE_EMPTY_(qptr) (qptr->data_count == 0)
// is the queue full?
#define _MERRY_IS_QUEUE_FULL_(qptr) (qptr->data_count == qptr->node_count)

#define _MERRY_QUEUE_INIT_(queue_name, qptr)             \
    do                                                   \
    {                                                    \
        qptr = (queue_name *)malloc(sizeof(queue_name)); \
    } while (0)
// checking if the pointer was initialized is the caller's job

#define _MERRY_QUEUE_CREATE_NODES_(qptr, node_name, create_node_count, ret, val_type) \
    do                                                                                \
    {                                                                                 \
        qptr->node_count = (create_node_count);                                       \
        qptr->data_count = 0;                                                         \
        qptr->head = (node_name *)malloc(sizeof(node_name));                          \
        if (qptr->head == NULL)                                                       \
        {                                                                             \
            ret = mfalse;                                                             \
            break;                                                                    \
        }                                                                             \
        node_name *current = (qptr)->head;                                            \
        for (msize_t i = 0; i < (create_node_count); i++)                             \
        {                                                                             \
            current->value = (val_type *)malloc(sizeof((val_type)));                  \
            if (current->value == NULL)                                               \
            {                                                                         \
                (ret) = mfalse;                                                       \
                break;                                                                \
            }                                                                         \
            current->next = (node_name *)malloc(sizeof(node_name));                   \
            if (current->next == NULL)                                                \
            {                                                                         \
                (ret) = mfalse;                                                       \
                break;                                                                \
            }                                                                         \
            current = current->next;                                                  \
        }                                                                             \
        current->next = (qptr)->head;                                                 \
        (qptr->tail) = (qptr)->head; /*The queue is empty*/                           \
        ret = mtrue;                                                                  \
    } while (0)

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
    } while (0)

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
    } while (0)

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
                    free(current->value);                \
                free(current);                           \
            }                                            \
            current = next;                              \
        }                                                \
        (qptr)->head = NULL;                             \
        (qptr)->tail = NULL;                             \
        (qptr)->node_count = 0;                          \
        free(qptr);                                      \
    } while (0)

// Emergency add to first
#define _MERRY_QUEUE_PANIC_PUSH_(qptr, val)                     \
    do                                                          \
    { /*This is a emergency push and so it takes top priority*/ \
        *(qptr->head->value) = val;                             \
    } while (0)

// NO POINTER
#define _MERRY_CREATE_QUEUE_NODE_NOPTR_(type, name) \
    struct name                                     \
    {                                               \
        type value;                                 \
        struct name *next;                          \
    }

// suffix could be anything but for the VM it would be something like CharQueue, IntQueue
#define _MERRY_CREATE_QUEUE_NOPTR_(name, node_name) _MERRY_CREATE_QUEUE_(name, node_name)

// is queue empty?
#define _MERRY_IS_QUEUE_EMPTY_NOPTR_(qptr) _MERRY_IS_QUEUE_EMPTY_(qptr)
// is the queue full?
#define _MERRY_IS_QUEUE_FULL_NOPTR_(qptr) _MERRY_IS_QUEUE_FULL_(qptr)

#define _MERRY_QUEUE_INIT_NOPTR_(queue_name, qptr) _MERRY_QUEUE_INIT_(queue_name, qptr)
// checking if the pointer was initialized is the caller's job

#define _MERRY_QUEUE_CREATE_NODES_NOPTR_(qptr, node_name, create_node_count, ret) \
    do                                                                            \
    {                                                                             \
        qptr->node_count = (create_node_count);                                   \
        qptr->data_count = 0;                                                     \
        qptr->head = (node_name *)malloc(sizeof(node_name));                      \
        if (qptr->head == NULL)                                                   \
        {                                                                         \
            ret = mfalse;                                                         \
            break;                                                                \
        }                                                                         \
        node_name *current = (qptr)->head;                                        \
        for (msize_t i = 0; i < (create_node_count); i++)                         \
        {                                                                         \
            current->next = (node_name *)malloc(sizeof(node_name));               \
            if (current->next == NULL)                                            \
            {                                                                     \
                (ret) = mfalse;                                                   \
                break;                                                            \
            }                                                                     \
            current = current->next;                                              \
        }                                                                         \
        current->next = (qptr)->head;                                             \
        (qptr->tail) = (qptr)->head; /*The queue is empty*/                       \
        ret = mtrue;                                                              \
    } while (0)

// push a value to the queue
#define _MERRY_QUEUE_PUSH_NOPTR_(qptr, pvalue, ret) \
    do                                              \
    {                                               \
        if (_MERRY_IS_QUEUE_FULL_(qptr))            \
        {                                           \
            ret = mfalse;                           \
            break;                                  \
        }                                           \
        (qptr)->tail->value = (pvalue);             \
        (qptr)->data_count++;                       \
        (qptr)->tail = (qptr)->tail->next;          \
        ret = mtrue;                                \
    } while (0)

// pop a value from the queue
// dest is not a pointer
#define _MERRY_QUEUE_POP_NOPTR_(qptr, dest, ret) \
    do                                           \
    {                                            \
        if (_MERRY_IS_QUEUE_EMPTY_(qptr))        \
        {                                        \
            ret = mfalse;                        \
            break;                               \
        }                                        \
        dest = (qptr)->head->value;              \
        (qptr)->head = (qptr)->head->next;       \
        (qptr)->data_count--;                    \
        ret = mtrue;                             \
    } while (0)

// free the queue
#define _MERRY_DESTROY_QUEUE_NOPTR_(qptr)                \
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
                free(current);                           \
            }                                            \
            current = next;                              \
        }                                                \
        (qptr)->head = NULL;                             \
        (qptr)->tail = NULL;                             \
        (qptr)->node_count = 0;                          \
        free(qptr);                                      \
    } while (0)

// Emergency add to first
#define _MERRY_QUEUE_PANIC_PUSH_NOPTR_(qptr, val)               \
    do                                                          \
    { /*This is a emergency push and so it takes top priority*/ \
        (qptr->head->value) = val;                              \
    } while (0)

#endif