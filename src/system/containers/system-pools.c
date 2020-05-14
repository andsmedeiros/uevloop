#include "uevloop/system/containers/system-pools.h"
#include "uevloop/portability/critical-section.h"

void uel_syspools_init(uel_syspools_t *pools){
    uel_objpool_init(
        &pools->event_pool,
        UEL_SYSPOOLS_EVENT_POOL_SIZE_LOG2N,
        sizeof(uel_event_t),
       UEL_OBJPOOL_BUFFERS_AT(event, pools)
    );
    uel_objpool_init(
        &pools->llist_node_pool,
        UEL_SYSPOOLS_LLIST_NODE_POOL_SIZE_LOG2N,
        sizeof(uel_llist_node_t),
       UEL_OBJPOOL_BUFFERS_AT(llist_node, pools)
    );
}

uel_event_t *uel_syspools_acquire_event(uel_syspools_t *pools){
    UEL_CRITICAL_ENTER;
    uel_event_t *event = (uel_event_t *)uel_objpool_acquire(&pools->event_pool);
    UEL_CRITICAL_EXIT;
    return event;
}

uel_llist_node_t *uel_syspools_acquire_llist_node(uel_syspools_t *pools){
    UEL_CRITICAL_ENTER;
    uel_llist_node_t *node = (uel_llist_node_t *)uel_objpool_acquire(&pools->llist_node_pool);
    UEL_CRITICAL_EXIT;
    return node;
}

bool uel_syspools_release_event(uel_syspools_t *pools, uel_event_t *event){
    UEL_CRITICAL_ENTER;
    bool released = uel_objpool_release(&pools->event_pool, (void *)event);
    UEL_CRITICAL_EXIT;
    return released;
}

bool uel_syspools_release_llist_node(uel_syspools_t *pools, uel_llist_node_t *node){
    UEL_CRITICAL_ENTER;
    bool released = uel_objpool_release(&pools->llist_node_pool, (void *)node);
    UEL_CRITICAL_EXIT;
    return released;
}
