#include "system-pools.h"

#include <stdlib.h>
#include "../../portability/critical-section.h"

void syspools_init(syspools_t *pools){
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

event_t *syspools_acquire_event(syspools_t *pools){
    UEVLOOP_CRITICAL_ENTER;
    event_t *event = (event_t *)objpool_acquire(&pools->event_pool);
    UEVLOOP_CRITICAL_EXIT;
    return event;
}

llist_node_t *syspools_acquire_llist_node(syspools_t *pools){
    UEVLOOP_CRITICAL_ENTER;
    llist_node_t *node = (llist_node_t *)objpool_acquire(&pools->llist_node_pool);
    UEVLOOP_CRITICAL_EXIT;
    return node;
}

bool syspools_release_event(syspools_t *pools, event_t *event){
    UEVLOOP_CRITICAL_ENTER;
    bool released = objpool_release(&pools->event_pool, (void *)event);
    UEVLOOP_CRITICAL_EXIT;
    return released;
}

bool syspools_release_llist_node(syspools_t *pools, llist_node_t *node){
    UEVLOOP_CRITICAL_ENTER;
    bool released = objpool_release(&pools->llist_node_pool, (void *)node);
    UEVLOOP_CRITICAL_EXIT;
    return released;
}
