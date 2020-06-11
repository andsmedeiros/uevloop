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

    uel_autoptr_t objs[4];
    for (size_t i = 0; i < 4; i++) {
        uel_autoptr_t obj = uel_autopool_alloc(&pool);
        uelt_assert_pointer_not_null("obj", obj);
        uelt_assert_pointers_equal("obj", &test_pool_buffer[i], obj);
        uelt_assert_pointers_equal("*obj", &test_buffer[i], *obj);
        uelt_assert_pointers_equal(
            "((struct uel_autoptr *)obj)->source",
            &pool.autoptr_pool,
            ((struct uel_autoptr *)obj)->source
        );
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

static void *construct(void *context, void *params) {
    struct test_obj * obj = (struct test_obj *)params;
    obj->c = 'C';
    obj->i = 10;
    return NULL;
}
static void *destruct(void *context, void *params) {
    struct test_obj * obj = (struct test_obj *)params;
    obj->c = 'D';
    obj->i = 1;
    return NULL;
}

static char *should_construct_and_destruct_objects() {
    UEL_DECLARE_AUTOPOOL_BUFFERS(struct test_obj, 2, test);
    uel_autopool_t pool;
    uel_autopool_init(&pool, 2, sizeof(struct test_obj), UEL_AUTOPOOL_BUFFERS(test));

    uel_autopool_set_constructor(&pool, uel_closure_create(construct, NULL));
    uel_autopool_set_destructor(&pool, uel_closure_create(destruct, NULL));

    struct test_obj **obj = (struct test_obj **)uel_autopool_alloc(&pool);

    uelt_assert_equals("(**obj).c", 'C', (**obj).c, "%c");
    uelt_assert_ints_equal("(**obj).i", 10, (**obj).i);

    uel_autoptr_dealloc((uel_autoptr_t)obj);

    uelt_assert_equals("(**obj).c", 'D', (**obj).c, "%c");
    uelt_assert_ints_equal("(**obj).i", 1, (**obj).i);

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
    uelt_run_test(
        "should correctly construct and destruct objects",
        should_construct_and_destruct_objects
    );

    return NULL;
}
