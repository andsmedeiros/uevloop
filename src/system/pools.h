#ifndef POOLS_H
#define	POOLS_H

#include "../utils/object-pool.h"
#include "../utils/linked-list.h"
#include "event.h"

typedef struct pools pools_t;
struct pools{

    /* event pool */
    #define EVENT_POOL_SIZE_LOG2N   (7) //128 events
    DECLARE_OBJPOOL_BUFFERS(event_t, EVENT_POOL_SIZE_LOG2N, event);
    objpool_t event_pool;

    /* linked list node pool */
    #define LLIST_NODE_POOL_SIZE_LOG2N    (7) //128 nodes
    DECLARE_OBJPOOL_BUFFERS(llist_node_t, LLIST_NODE_POOL_SIZE_LOG2N, llist_node);
    objpool_t llist_node_pool;

};

void pools_init(pools_t *pools);

#endif	/* POOLS_H */
