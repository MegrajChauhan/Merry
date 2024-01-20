#include "../merry_inst_queue.h"

// MerryInstQueue *merry_inst_queue_init(msize_t queue_len)
// {
//     MerryInstQueue *queue = (MerryInstQueue *)malloc(sizeof(MerryInstQueue));
//     if (queue == NULL)
//         return NULL;
//     mbool_t ret = mtrue;
//     _MERRY_QUEUE_CREATE_NODES_(queue, MerryInstQueueNode, queue_len, ret)
// }