#include "system-pools.h"

#include <stdlib.h>

#include "system/containers/system-pools.h"
#include "../../uelt.h"

static char *should_init_syspools(){
    uel_syspools_t pools;
    uel_syspools_init(&pools);

    uelt_assert_pointers_equal(
        "event_pool.buffer",
        pools.event_pool_buffer,
        pools.event_pool.buffer
    );
    uelt_assert_pointers_equal(
        "event_pool.queue.buffer",
        pools.event_pool_queue_buffer,
        pools.event_pool.queue.buffer
    );
    uelt_assert_ints_equal(
        "pool.queue.size",
        UEL_SYSPOOLS_EVENT_POOL_SIZE,
        pools.event_pool.queue.size
    );
    uelt_assert_ints_equal(
        "pool.queue.count",
        UEL_SYSPOOLS_EVENT_POOL_SIZE,
        pools.event_pool.queue.count
    );
    uelt_assert_pointers_equal(
        "llist_node_pool.buffer",
        pools.llist_node_pool_buffer,
        pools.llist_node_pool.buffer
    );
    uelt_assert_pointers_equal(
        "llist_node_pool.queue.buffer",
        pools.llist_node_pool_queue_buffer,
        pools.llist_node_pool.queue.buffer
    );
    uelt_assert_ints_equal(
        "llist_node_pool.queue.size",
        UEL_SYSPOOLS_LLIST_NODE_POOL_SIZE,
        pools.llist_node_pool.queue.size
    );
    uelt_assert_ints_equal(
        "llist_node_pool.queue.count",
        UEL_SYSPOOLS_LLIST_NODE_POOL_SIZE,
        pools.llist_node_pool.queue.count
    );

    return NULL;
}

static char *should_acquire_objects(){
    uel_syspools_t pools;
    uel_syspools_init(&pools);

    uel_event_t *event = uel_syspools_acquire_event(&pools);
    uelt_assert_pointer_not_null("acquired event must not be null", event);

    uel_llist_node_t *node = uel_syspools_acquire_llist_node(&pools);
    uelt_assert_pointer_not_null("acquired llist node must not be null", node);

    return NULL;
}

static char *should_release_objects(){
    uel_syspools_t pools;
    uel_syspools_init(&pools);

    uel_event_t *event = uel_syspools_acquire_event(&pools);
    uel_llist_node_t *node = uel_syspools_acquire_llist_node(&pools);

    bool event_released = uel_syspools_release_event(&pools, event);
    uelt_assert("event must had been successfully released", event_released);

    bool node_released = uel_syspools_release_llist_node(&pools, node);
    uelt_assert("linked list node must had been successfully released", node_released);

    return NULL;
}

char *uel_syspools_run_tests(){
    uelt_run_test("should correctly initiase system pools", should_init_syspools);
    uelt_run_test("should correctly acquire objects", should_acquire_objects);
    uelt_run_test("should correctly release objects", should_release_objects);

    return NULL;
}
