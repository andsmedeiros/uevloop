#include "pools.h"

#include <stdlib.h>

#include "system/pools.h"
#include "../minunit.h"

static char *should_init_pools(){
    pools_t pools;
    pools_init(&pools);

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

char *pools_run_tests(){
    mu_run_test("should correctly initiase system pools", should_init_pools);

    return NULL;
}
