#include "object-pool.h"

#include <stdlib.h>

#include "utils/object-pool.h"
#include "../minunit.h"

typedef struct{
    char character;
    uintptr_t integer;
    float rational;
} object_t;

static char *should_init_objpool(){
    DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    objpool_t pool;
    objpool_init(&pool, 3, sizeof(object_t), OBJPOOL_BUFFERS(main));

    mu_assert_pointers_equal("pool.buffer", main_pool_buffer, pool.buffer);
    mu_assert_pointers_equal(
        "pool.queue.buffer",
        main_pool_queue_buffer,
        pool.queue.buffer
    );
    mu_assert_ints_equal("pool.queue.size", 8, pool.queue.size);
    mu_assert_ints_equal("pool.queue.count", 8, pool.queue.count);

    return NULL;
}

static char *should_acquire_objects(){
    DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    objpool_t pool;
    objpool_init(&pool, 3, sizeof(object_t), OBJPOOL_BUFFERS(main));

    for(uintptr_t i = 0; i < 8; i++){
        object_t *obj = (object_t *)objpool_acquire(&pool);
        mu_assert_pointers_equal(
            "objpool_acquire()",
            &main_pool_buffer[i * sizeof(object_t)],
            obj
        );
    }

    return NULL;
}

static char *should_release_objects(){
    DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    objpool_t pool;
    objpool_init(&pool, 3, sizeof(object_t), OBJPOOL_BUFFERS(main));

    object_t *objects[8] = {0};

    for(uintptr_t i = 0; i < 8; i++){
        objects[i] = (object_t *)objpool_acquire(&pool);
        objects[i]->character = 'a' + i;
        objects[i]->integer = i * 10;
        objects[i]->rational = i / 2.0;
    }

    for(uintptr_t i = 0; i < 8; i++){
        objpool_release(&pool, objects[i]);
        mu_assert_equals("objects[i]->character", ('a' + i), objects[i]->character, "%c");
        mu_assert_ints_equal("objects[i]->integer", 10 * i, objects[i]->integer);
        mu_assert_equals("objects[i]->rational", i / 2.0, objects[i]->rational, "%f");

        object_t *object = (object_t *)&pool.buffer[i * sizeof(object_t)];
        mu_assert_equals("obj->character", ('a' + i), object->character, "%c");
        mu_assert_ints_equal("obj->integer", 10 * i, object->integer);
        mu_assert_equals("obj->rational", i / 2.0, object->rational, "%f");
    }
    mu_assert_ints_equal("pool.queue.count", 8, pool.queue.count);

    return NULL;
}

static char *should_detect_when_pool_is_empty(){
    DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    objpool_t pool;
    objpool_init(&pool, 3, sizeof(object_t), OBJPOOL_BUFFERS(main));

    for(uintptr_t i = 0; i < 7; i++){
        objpool_acquire(&pool);
    }
    mu_assert_not(
        "objpool_is_empty() must had returned false before last acquire()",
        objpool_is_empty(&pool)
    );

    object_t *obj = (object_t *)objpool_acquire(&pool);
    mu_assert(
        "objpool_is_empty() must had returned true after last acquire()",
        objpool_is_empty(&pool)
    );

    objpool_release(&pool, obj);
    mu_assert_not(
        "objpool_is_empty() must had returned false after release()",
        objpool_is_empty(&pool)
    );

    return NULL;
}

char *objpool_run_tests(){

    mu_run_test("should correctly initialise object pool", should_init_objpool);
    mu_run_test(
        "should correctly acquire objects from the pool",
        should_acquire_objects
    );
    mu_run_test(
        "should correctly release objects to the pool",
        should_release_objects
    );
    mu_run_test(
        "should correctly detect when a pool is empty",
        should_detect_when_pool_is_empty
    );

    return NULL;
}
