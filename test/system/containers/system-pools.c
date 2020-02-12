#include "system-pools.h"

#include <stdlib.h>

#include "system/containers/system-pools.h"
#include "../../minunit.h"

static char *should_init_syspools(){
    syspools_t pools;
    syspools_init(&pools);

    mu_assert_pointers_equal(
        "event_pool.buffer",
        pools.event_pool_buffer,
        pools.event_pool.buffer
    );
    mu_assert_pointers_equal(
        "event_pool.queue.buffer",
        pools.event_pool_queue_buffer,
        pools.event_pool.queue.buffer
    );
    mu_assert_ints_equal(
        "pool.queue.size",
        1<<EVENT_POOL_SIZE_LOG2N,
        pools.event_pool.queue.size
    );
    mu_assert_ints_equal(
        "pool.queue.count",
        1<<EVENT_POOL_SIZE_LOG2N,
        pools.event_pool.queue.count
    );
    mu_assert_pointers_equal(
        "llist_node_pool.buffer",
        pools.llist_node_pool_buffer,
        pools.llist_node_pool.buffer
    );
    mu_assert_pointers_equal(
        "llist_node_pool.queue.buffer",
        pools.llist_node_pool_queue_buffer,
        pools.llist_node_pool.queue.buffer
    );
    mu_assert_ints_equal(
        "llist_node_pool.queue.size",
        1<<LLIST_NODE_POOL_SIZE_LOG2N,
        pools.llist_node_pool.queue.size
    );
    mu_assert_ints_equal(
        "llist_node_pool.queue.count",
        1<<LLIST_NODE_POOL_SIZE_LOG2N,
        pools.llist_node_pool.queue.count
    );

    return NULL;
}

static char *should_acquire_objects(){
    syspools_t pools;
    syspools_init(&pools);

    event_t *event = syspools_acquire_event(&pools);
    mu_assert_pointer_not_null("acquired event must not be null", event);

    llist_node_t *node = syspools_acquire_llist_node(&pools);
    mu_assert_pointer_not_null("acquired llist node must not be null", node);

    return NULL;
}

static char *should_release_objects(){
    syspools_t pools;
    syspools_init(&pools);

    event_t *event = syspools_acquire_event(&pools);
    llist_node_t *node = syspools_acquire_llist_node(&pools);

    bool event_released = syspools_release_event(&pools, event);
    mu_assert("event must had been successfully released", event_released);

    bool node_released = syspools_release_llist_node(&pools, node);
    mu_assert("linked list node must had been successfully released", node_released);

    return NULL;
}

char *syspools_run_tests(){
    mu_run_test("should correctly initiase system pools", should_init_syspools);
    mu_run_test("should correctly acquire objects", should_acquire_objects);
    mu_run_test("should correctly release objects", should_release_objects);

    return NULL;
}
