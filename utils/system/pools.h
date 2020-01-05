/* 
 * File:   pools.h
 * Author: kazeshi
 *
 * Created on January 4, 2020, 4:19 PM
 */

#ifndef POOLS_H
#define	POOLS_H

#include "../utils/object-pool.h"
#include "../utils/linked-list.h"
#include "event.h"

typedef enum pool_id pool_id_t;
enum pool_id {
    EVENT_POOL,
    LLIST_NODE_POOL
};

typedef struct pools pools_t;
struct pools{
    
    /* event pool */
    #define EVENT_POOL_SIZE_LOG2N   (6) //64 events
    DECLARE_OBJPOOL_BUFFERS(event_t, EVENT_POOL_SIZE_LOG2N, event);
    objpool_t event_pool;
    
    /* linked list node pool */
    #define LLIST_NODE_POOL_SIZE_LOG2N    (5) //32 nodes
    DECLARE_OBJPOOL_BUFFERS(llist_node_t, LLIST_NODE_POOL_SIZE_LOG2N, llist_node);
    objpool_t llist_node_pool;
    
};

void pools_init(pools_t *pools);
objpool_t *pools_get(pools_t *pools, pool_id_t);

#endif	/* POOLS_H */

