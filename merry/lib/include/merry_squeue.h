#ifndef _MERRY_SQUEUE_
#define _MERRY_SQUEUE_

#include "../../../utils/merry_types.h"
#include <stdlib.h>

// Unlike the other queue squeue(simple queue) doesn't use fancy structures
#define _MERRY_CREATE_SQUEUE_(type, name) \
    typedef struct name                   \
    {                                     \
        type *queue;                      \
        msize_t qlen;                     \
        msize_t on_queue;                 \
        msize_t head, tail;               \
    }name

#define _MERRY_IS_SQUEUE_FULL_(qptr) (qptr->on_queue == qptr->qlen)
#define _MERRY_IS_SQUEUE_EMPTY_(qptr) (qptr->on_queue == 0)

#define _MERRY_SQUEUE_INIT_(qptr)                            \
    do                                                       \
    {                                                        \
        qptr = (typeof(qptr) *)malloc(sizeof(typeof(qptr))); \
    } while (0);

#define _MERRY_SQUEUE_POPULATE_(qptr, size, type)          \
    do                                                     \
    {                                                      \
        qptr->queue = (type *)malloc(sizeof(type) * size); \
        qptr->qlen = size;                                 \
        qptr->on_queue = 0;                                \
        qptr->head = 0;                                    \
        qptr->tail = 0;                                    \
    } while (0);

#define _MERRY_SQUEUE_DESTROY_(qptr) \
    do                               \
    {                                \
        if (qptr == NULL)            \
            break;                   \
        if (qptr->queue != NULL)     \
            free(qptr->queue);       \
        free(qptr);                  \
    } while (0);

#define _MERRY_SQUEUE_PUSH_(qptr, value, ret)       \
    do                                              \
    {                                               \
        if (_MERRY_IS_SQUEUE_FULL_(qptr))           \
        {                                           \
            ret = mfalse;                           \
            break;                                  \
        }                                           \
        qptr->queue[qptr->tail] = value;            \
        qptr->tail = (qptr->tail + 1) % qptr->qlen; \
        qptr->on_queue++;                           \
    } while (0);

#define _MERRY_SQUEUE_POP_(qptr, value, ret)        \
    do                                              \
    {                                               \
        if (_MERRY_IS_SQUEUE_EMPTY_(qptr))          \
        {                                           \
            ret = mfalse;                           \
            break;                                  \
        }                                           \
        value = qptr->queue[qptr->head];            \
        qptr->head = (qptr->head + 1) % qptr->qlen; \
        qptr->on_queue--;                           \
    } while (0);

#endif