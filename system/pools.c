#include "pools.h"
#include <stdlib.h>

void pools_init(pools_t *pools){
    objpool_init(
        &pools->event_pool, 
        EVENT_POOL_SIZE_LOG2N, 
        sizeof(event_t), 
        OBJPOOL_BUFFERS_AT(event, pools)
    );
    objpool_init(
        &pools->llist_node_pool,
        LLIST_NODE_POOL_SIZE_LOG2N,
        sizeof(llist_node_t),
        OBJPOOL_BUFFERS_AT(llist_node, pools)
    );
}

objpool_t *pools_get(pools_t *pools, pool_id_t pool){
    switch (pool){
        case EVENT_POOL: 
            return &pools->event_pool;
            break;
        case LLIST_NODE_POOL:
            return &pools->llist_node_pool;
            break;
        default:
            return NULL;
            break;
    }
}