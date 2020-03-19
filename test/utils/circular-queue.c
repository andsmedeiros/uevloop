#include "circular-queue.h"

#include <stdint.h>
#include <math.h>

#include "utils/circular-queue.h"
#include "../uelt.h"

#define BUFFER_SIZE_LOG2N   (5)
#define BUFFER_SIZE         (1<<BUFFER_SIZE_LOG2N)

static char *should_init(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uelt_assert_pointers_equal("cqueue.buffer", &buffer, queue.buffer);
    uelt_assert_ints_equal("cqueue.size", 32, queue.size);
    uelt_assert_ints_equal("cqueue.mask", 31, queue.mask);
    uelt_assert_pointer_null("cqueue.tail", queue.tail);
    uelt_assert_int_zero("cqueue.count", queue.count);

    return NULL;
}

static char *should_do_partial_clear(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uel_cqueue_clear(&queue, false);
    uelt_assert_int_zero("cqueue.tail", queue.tail);
    uelt_assert_int_zero("cqueue.count", queue.count);
    uelt_assert_pointer_not_null("cqueue.buffer", queue.buffer);
    uelt_assert_int_not_zero("cqueue.size", queue.size);
    uelt_assert_int_not_zero("cqueue.size", queue.mask);

    return NULL;
}

static char *should_do_full_clear(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uel_cqueue_clear(&queue, true);
    uelt_assert_int_zero("cqueue.tail", queue.tail);
    uelt_assert_int_zero("cqueue.count", queue.count);
    uelt_assert_pointer_null("cqueue.buffer", queue.buffer);
    uelt_assert_int_zero("cqueue.size", queue.size);
    uelt_assert_int_zero("cqueue.size", queue.mask);

    return NULL;
}

static char *should_push_element(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t elements[3] = { 213, 13, 75 }, values[3] = { 213, 13, 75 };

    uintptr_t i;
    for(i = 0; i < 3; i++){
        uel_cqueue_push(&queue, (void *)&elements[i]);
        uelt_assert_ints_equal("cqueue.count", i + 1, queue.count);
        uelt_assert_pointers_equal(
            "uel_cqueue_peek_head()",
            &elements[i],
            uel_cqueue_peek_head(&queue)
        );
        uelt_assert_ints_equal(
            "uel_cqueue_peek_head()",
            values[i],
            *(uint8_t *)uel_cqueue_peek_head(&queue)
        );
    }

    return NULL;
}

static char *should_pop_element(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);


    uint8_t *element = (uint8_t *)uel_cqueue_pop(&queue);
    uelt_assert_pointer_null("uel_cqueue_pop() with empty queue", element);

    uint8_t elements[3] = { 213, 13, 75 }, values[3] = { 213, 13, 75 };

    uintptr_t i;
    for(i = 0; i < 3; i++){
        uel_cqueue_push(&queue, (void *)&elements[i]);
    }
    for(i = 0; i < 3; i++){
        element = (uint8_t *)uel_cqueue_pop(&queue);
        uelt_assert_pointers_equal("uel_cqueue_pop()", &elements[i], element);
        uelt_assert_ints_equal("uel_cqueue_pop()", values[i], *element);
    }

    return NULL;
}

static char *should_peek_tail(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t *element = (uint8_t *)uel_cqueue_peek_tail(&queue);
    uelt_assert_pointer_null("uel_cqueue_peek_tail() with empty queue", element);

    uint8_t elements[3] = { 213, 13, 75 }, value = 213;

    uintptr_t i;
    for(i = 0; i < 3; i++){
        uel_cqueue_push(&queue, (void *)&elements[i]);
        element = (uint8_t *)uel_cqueue_peek_tail(&queue);
        uelt_assert_pointers_equal("uel_cqueue_peek_tail()", &elements[0], element);
        uelt_assert_ints_equal("uel_cqueue_peek_tail()", value, *element);
    }

    return NULL;
}

static char *should_peek_head(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t *element = (uint8_t *)uel_cqueue_peek_head(&queue);
    uelt_assert_pointer_null("uel_cqueue_peek_head() with empty queue", element);

    uint8_t elements[3] = { 213, 13, 75 }, values[3] = { 213, 13, 75 };

    uintptr_t i;
    for(i = 0; i < 3; i++){
        uel_cqueue_push(&queue, (void *)&elements[i]);
        element = (uint8_t *)uel_cqueue_peek_head(&queue);
        uelt_assert_pointers_equal("uel_cqueue_peek_head()", &elements[i], element);
        uelt_assert_ints_equal("uel_cqueue_peek_head()", values[i], *element);
    }

    return NULL;
}

static char *should_detect_when_full(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t elements[32];
    uintptr_t i;

    for(i = 0; i < 32; i++) elements[i] = i;
    for(i = 0; i < 31; i++){
        uel_cqueue_push(&queue, (void *)&elements[i]);
        uelt_assert_not(
            "uel_cqueue_is_full() before filling, expected to be false",
            uel_cqueue_is_full(&queue)
        );
    }

    uel_cqueue_push(&queue, &elements[31]);
    uelt_assert(
        "uel_cqueue_is_full() after filling, expected to be true",
        uel_cqueue_is_full(&queue)
    );

    uel_cqueue_pop(&queue);
    uelt_assert_not(
        "uel_cqueue_is_full() after popping one element, expected to be false",
        uel_cqueue_is_full(&queue)
    );

    return NULL;
}

static char *should_detect_when_empty(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uelt_assert(
        "uel_cqueue_is_empty() before pushing, expected to be true",
        uel_cqueue_is_empty(&queue)
    );

    uint8_t element = 213;
    uel_cqueue_push(&queue, &element);
    uelt_assert_not(
        "uel_cqueue_is_empty() after pushing, expected to be false",
        uel_cqueue_is_empty(&queue)
    );

    uel_cqueue_pop(&queue);
    uelt_assert(
        "uel_cqueue_is_empty() after emptying, expected to be true",
        uel_cqueue_is_empty(&queue)
    );

    return NULL;
}

static char *should_wrap_on_buffer_limit(){
    uel_cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    uel_cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    queue.tail = BUFFER_SIZE - 3;

    uint8_t elements[3] = { 213, 13, 75 };
    uintptr_t i;
    for(i = 0; i < 3; i++){
        uel_cqueue_push(&queue, (void *)&elements[i]);
    }
    uelt_assert_ints_equal("*cqueue.buffer[0]", 75, *(uint8_t *)queue.buffer[0]);

    return NULL;
}

char * uel_cqueue_run_tests(){
    uelt_run_test("should init circular queue with blank fields", should_init);
    uelt_run_test(
        "should partially clear queue correctly",
        should_do_partial_clear
    );
    uelt_run_test(
        "should fully clear queue correctly",
        should_do_full_clear
    );
    uelt_run_test(
        "should correctly push elements in the queue",
        should_push_element
    );
    uelt_run_test(
        "should correctly pop elements from the queue",
        should_pop_element
    );
    uelt_run_test(
        "should correctly peek on the queue tail",
        should_peek_tail
    );
    uelt_run_test(
        "should correctly peek on the queue head",
        should_peek_head
    );
    uelt_run_test(
        "should correctly detect when the queue is full",
        should_detect_when_full
    );
    uelt_run_test(
        "should correctly detect when the queue is empty",
        should_detect_when_empty
    );
    uelt_run_test(
        "should correctly wrap over the buffer end when it is reached",
        should_wrap_on_buffer_limit
    );
    return NULL;
}

#undef BUFFER_SIZE_LOG2N
#undef BUFFER_SIZE
