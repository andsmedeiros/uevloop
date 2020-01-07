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

static char *should_get_pools(){
    pools_t pools;
    pools_init(&pools);

    objpool_t *event_pool = pools_get(&pools, EVENT_POOL);
    mu_assert_pointers_equal("pools_get(EVENT_POOL)", &pools.event_pool, event_pool);

    objpool_t *llist_node_pool = pools_get(&pools, LLIST_NODE_POOL);
    mu_assert_pointers_equal("pools_get(EVENT_POOL)", &pools.llist_node_pool, llist_node_pool);

    objpool_t *null_pool = pools_get(&pools, (pool_id_t)(100));
    mu_assert_pointer_null("pools_get(INVALID)", null_pool);

    return NULL;
}

char *pools_run_tests(){
    mu_run_test("should correctly initiase system pools", should_init_pools);
    mu_run_test("should correctly get system pools by their ID", should_get_pools);

    return NULL;
}
