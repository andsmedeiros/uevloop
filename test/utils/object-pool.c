#include "object-pool.h"

#include <stdlib.h>

#include "utils/object-pool.h"
#include "../uelt.h"

typedef struct{
    char character;
    uintptr_t integer;
    float rational;
} object_t;

static char *should_init_objpool(){
    UEL_DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    uel_objpool_t pool;
    uel_objpool_init(&pool, 3, sizeof(object_t),UEL_OBJPOOL_BUFFERS(main));

    uelt_assert_pointers_equal("pool.buffer", main_pool_buffer, pool.buffer);
    uelt_assert_pointers_equal(
        "pool.queue.buffer",
        main_pool_queue_buffer,
        pool.queue.buffer
    );
    uelt_assert_ints_equal("pool.queue.size", 8, pool.queue.size);
    uelt_assert_ints_equal("pool.queue.count", 8, pool.queue.count);

    return NULL;
}

static char *should_acquire_objects(){
    UEL_DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    uel_objpool_t pool;
    uel_objpool_init(&pool, 3, sizeof(object_t),UEL_OBJPOOL_BUFFERS(main));

    for(uintptr_t i = 0; i < 8; i++){
        object_t *obj = (object_t *)uel_objpool_acquire(&pool);
        uelt_assert_pointers_equal(
            "uel_objpool_acquire()",
            &main_pool_buffer[i * sizeof(object_t)],
            obj
        );
    }

    return NULL;
}

static char *should_release_objects(){
    UEL_DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    uel_objpool_t pool;
    uel_objpool_init(&pool, 3, sizeof(object_t),UEL_OBJPOOL_BUFFERS(main));

    object_t *objects[8] = {0};

    for(uintptr_t i = 0; i < 8; i++){
        objects[i] = (object_t *)uel_objpool_acquire(&pool);
        objects[i]->character = 'a' + i;
        objects[i]->integer = i * 10;
        objects[i]->rational = i / 2.0;
    }

    for(uintptr_t i = 0; i < 8; i++){
        uel_objpool_release(&pool, objects[i]);
        uelt_assert_equals("objects[i]->character", ('a' + i), objects[i]->character, "%c");
        uelt_assert_ints_equal("objects[i]->integer", 10 * i, objects[i]->integer);
        uelt_assert_equals("objects[i]->rational", i / 2.0, objects[i]->rational, "%f");

        object_t *object = (object_t *)&pool.buffer[i * sizeof(object_t)];
        uelt_assert_equals("obj->character", ('a' + i), object->character, "%c");
        uelt_assert_ints_equal("obj->integer", 10 * i, object->integer);
        uelt_assert_equals("obj->rational", i / 2.0, object->rational, "%f");
    }
    uelt_assert_ints_equal("pool.queue.count", 8, pool.queue.count);

    return NULL;
}

static char *should_detect_when_pool_is_empty(){
    UEL_DECLARE_OBJPOOL_BUFFERS(object_t, 3, main);
    uel_objpool_t pool;
    uel_objpool_init(&pool, 3, sizeof(object_t),UEL_OBJPOOL_BUFFERS(main));

    for(uintptr_t i = 0; i < 7; i++){
        uel_objpool_acquire(&pool);
    }
    uelt_assert_not(
        "uel_objpool_is_empty() must had returned false before last acquire()",
        uel_objpool_is_empty(&pool)
    );

    object_t *obj = (object_t *)uel_objpool_acquire(&pool);
    uelt_assert(
        "uel_objpool_is_empty() must had returned true after last acquire()",
        uel_objpool_is_empty(&pool)
    );

    uel_objpool_release(&pool, obj);
    uelt_assert_not(
        "uel_objpool_is_empty() must had returned false after release()",
        uel_objpool_is_empty(&pool)
    );

    return NULL;
}

char *objpool_run_tests(){

    uelt_run_test("should correctly initialise object pool", should_init_objpool);
    uelt_run_test(
        "should correctly acquire objects from the pool",
        should_acquire_objects
    );
    uelt_run_test(
        "should correctly release objects to the pool",
        should_release_objects
    );
    uelt_run_test(
        "should correctly detect when a pool is empty",
        should_detect_when_pool_is_empty
    );

    return NULL;
}
