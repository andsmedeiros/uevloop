#include "automatic-pool.h"
#include "../uelt.h"

#include <uevloop/utils/automatic-pool.h>
#include <stddef.h>

struct test_obj {
    int i;
    char c;
};

static char *should_initialise_autopool() {
    UEL_DECLARE_AUTOPOOL_BUFFERS(struct test_obj, 2, test);
    uel_autopool_t pool;
    uel_autopool_init(&pool, 2, sizeof(struct test_obj), UEL_AUTOPOOL_BUFFERS(test));

    uelt_assert_pointers_equal(
        "pool.autoptr_pool.buffer",
        test_pool_buffer,
        pool.autoptr_pool.buffer
    );
    uelt_assert_pointers_equal(
        "pool.autoptr_pool.queue.buffer",
        test_pool_queue_buffer,
        pool.autoptr_pool.queue.buffer
    );
    uelt_assert_ints_equal(
        "pool.autoptr_pool.queue.size",
        4,
        pool.autoptr_pool.queue.size
    );
    uelt_assert_ints_equal(
        "pool.autoptr_pool.queue.count",
        4,
        pool.autoptr_pool.queue.count
    );

    return NULL;
}

static char *should_manage_objects() {
    UEL_DECLARE_AUTOPOOL_BUFFERS(struct test_obj, 2, test);
    uel_autopool_t pool;
    uel_autopool_init(&pool, 2, sizeof(struct test_obj), UEL_AUTOPOOL_BUFFERS(test));

    uelt_assert_not("pool isn't empty", uel_autopool_is_empty(&pool));

    uel_autoptr_t *objs[4];
    for (size_t i = 0; i < 4; i++) {
        uel_autoptr_t *obj = uel_autopool_alloc(&pool);
        uelt_assert_pointers_equal("obj", &test_pool_buffer[i], obj);
        uelt_assert_pointers_equal("obj->object", &test_buffer[i], obj->object);
        uelt_assert_pointers_equal("obj->source", &pool.autoptr_pool, obj->source);
        uelt_assert_pointers_equal("*(void **)obj", &test_buffer[i], *(void **)obj);
        objs[i] = obj;
    }

    uelt_assert_int_zero(
        "pool.autoptr_pool.queue.count",
        pool.autoptr_pool.queue.count
    );
    uelt_assert("pool is empty", uel_autopool_is_empty(&pool));

    for (size_t i = 0; i < 4; i++) {
        uelt_assert_ints_equal(
            "pool.autoptr_pool.queue.count",
            i,
            pool.autoptr_pool.queue.count
        );
        uel_autoptr_dealloc(objs[i]);
    }
    uelt_assert_ints_equal(
        "pool.autoptr_pool.queue.count",
        4,
        pool.autoptr_pool.queue.count
    );

    return NULL;
}

char *uel_autopool_run_tests(){

    uelt_run_test(
        "should correctly initialise an autopool",
        should_initialise_autopool
    );
    uelt_run_test(
        "should correctly manage objects' lifetime",
        should_manage_objects
    );

    return NULL;
}
