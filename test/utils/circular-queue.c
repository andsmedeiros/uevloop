#include "circular-queue.h"

#include <stdint.h>
#include <math.h>

#include "utils/circular-queue.h"
#include "../minunit.h"

#define BUFFER_SIZE_LOG2N   (5)
#define BUFFER_SIZE         (1<<BUFFER_SIZE_LOG2N)

static char *should_init(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    mu_assert_pointers_equal("cqueue.buffer", &buffer, queue.buffer);
    mu_assert_ints_equal("cqueue.size", 32, queue.size);
    mu_assert_ints_equal("cqueue.mask", 31, queue.mask);
    mu_assert_pointer_null("cqueue.tail", queue.tail);
    mu_assert_int_zero("cqueue.count", queue.count);

    return NULL;
}

static char *should_do_partial_clear(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    cqueue_clear(&queue, false);
    mu_assert_int_zero("cqueue.tail", queue.tail);
    mu_assert_int_zero("cqueue.count", queue.count);
    mu_assert_pointer_not_null("cqueue.buffer", queue.buffer);
    mu_assert_int_not_zero("cqueue.size", queue.size);
    mu_assert_int_not_zero("cqueue.size", queue.mask);

    return NULL;
}

static char *should_do_full_clear(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    cqueue_clear(&queue, true);
    mu_assert_int_zero("cqueue.tail", queue.tail);
    mu_assert_int_zero("cqueue.count", queue.count);
    mu_assert_pointer_null("cqueue.buffer", queue.buffer);
    mu_assert_int_zero("cqueue.size", queue.size);
    mu_assert_int_zero("cqueue.size", queue.mask);

    return NULL;
}

static char *should_push_element(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t elements[3] = { 213, 13, 75 }, values[3] = { 213, 13, 75 };

    uintptr_t i;
    for(i = 0; i < 3; i++){
        cqueue_push(&queue, (void *)&elements[i]);
        mu_assert_ints_equal("cqueue.count", i + 1, queue.count);
        mu_assert_pointers_equal(
            "cqueue_peek_head()",
            &elements[i],
            cqueue_peek_head(&queue)
        );
        mu_assert_ints_equal(
            "*cqueue_peek_head()",
            values[i],
            *(uint8_t *)cqueue_peek_head(&queue)
        );
    }

    return NULL;
}

static char *should_pop_element(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);


    uint8_t *element = (uint8_t *)cqueue_pop(&queue);
    mu_assert_pointer_null("cqueue_pop() with empty queue", element);

    uint8_t elements[3] = { 213, 13, 75 }, values[3] = { 213, 13, 75 };

    uintptr_t i;
    for(i = 0; i < 3; i++){
        cqueue_push(&queue, (void *)&elements[i]);
    }
    for(i = 0; i < 3; i++){
        element = (uint8_t *)cqueue_pop(&queue);
        mu_assert_pointers_equal("cqueue_pop()", &elements[i], element);
        mu_assert_ints_equal("*cqueue_pop()", values[i], *element);
    }

    return NULL;
}

static char *should_peek_tail(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t *element = (uint8_t *)cqueue_peek_tail(&queue);
    mu_assert_pointer_null("cqueue_peek_tail() with empty queue", element);

    uint8_t elements[3] = { 213, 13, 75 }, value = 213;

    uintptr_t i;
    for(i = 0; i < 3; i++){
        cqueue_push(&queue, (void *)&elements[i]);
        element = (uint8_t *)cqueue_peek_tail(&queue);
        mu_assert_pointers_equal("cqueue_peek_tail()", &elements[0], element);
        mu_assert_ints_equal("*cqueue_peek_tail()", value, *element);
    }

    return NULL;
}

static char *should_peek_head(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t *element = (uint8_t *)cqueue_peek_head(&queue);
    mu_assert_pointer_null("cqueue_peek_head() with empty queue", element);

    uint8_t elements[3] = { 213, 13, 75 }, values[3] = { 213, 13, 75 };

    uintptr_t i;
    for(i = 0; i < 3; i++){
        cqueue_push(&queue, (void *)&elements[i]);
        element = (uint8_t *)cqueue_peek_head(&queue);
        mu_assert_pointers_equal("cqueue_peek_head()", &elements[i], element);
        mu_assert_ints_equal("*cqueue_peek_head()", values[i], *element);
    }

    return NULL;
}

static char *should_detect_when_full(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    uint8_t elements[32];
    uintptr_t i;

    for(i = 0; i < 32; i++) elements[i] = i;
    for(i = 0; i < 31; i++){
        cqueue_push(&queue, (void *)&elements[i]);
        mu_assert_not(
            "cqueue_is_full() before filling, expected to be false",
            cqueue_is_full(&queue)
        );
    }

    cqueue_push(&queue, &elements[31]);
    mu_assert(
        "cqueue_is_full() after filling, expected to be true",
        cqueue_is_full(&queue)
    );

    cqueue_pop(&queue);
    mu_assert_not(
        "cqueue_is_full() after popping one element, expected to be false",
        cqueue_is_full(&queue)
    );

    return NULL;
}

static char *should_detect_when_empty(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    mu_assert(
        "cqueue_is_empty() before pushing, expected to be true",
        cqueue_is_empty(&queue)
    );

    uint8_t element = 213;
    cqueue_push(&queue, &element);
    mu_assert_not(
        "cqueue_is_empty() after pushing, expected to be false",
        cqueue_is_empty(&queue)
    );

    cqueue_pop(&queue);
    mu_assert(
        "cqueue_is_empty() after emptying, expected to be true",
        cqueue_is_empty(&queue)
    );

    return NULL;
}

static char *should_wrap_on_buffer_limit(){
    cqueue_t queue;
    void *buffer[BUFFER_SIZE];
    cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

    queue.tail = BUFFER_SIZE - 3;

    uint8_t elements[3] = { 213, 13, 75 };
    uintptr_t i;
    for(i = 0; i < 3; i++){
        cqueue_push(&queue, (void *)&elements[i]);
    }
    mu_assert_ints_equal("*cqueue.buffer[0]", 75, *(uint8_t *)queue.buffer[0]);

    return NULL;
}

char * cqueue_run_tests(){
    mu_run_test("should init circular queue with blank fields", should_init);
    mu_run_test(
        "should partially clear queue correctly",
        should_do_partial_clear
    );
    mu_run_test(
        "should fully clear queue correctly",
        should_do_full_clear
    );
    mu_run_test(
        "should correctly push elements in the queue",
        should_push_element
    );
    mu_run_test(
        "should correctly pop elements from the queue",
        should_pop_element
    );
    mu_run_test(
        "should correctly peek on the queue tail",
        should_peek_tail
    );
    mu_run_test(
        "should correctly peek on the queue head",
        should_peek_head
    );
    mu_run_test(
        "should correctly detect when the queue is full",
        should_detect_when_full
    );
    mu_run_test(
        "should correctly detect when the queue is empty",
        should_detect_when_empty
    );
    mu_run_test(
        "should correctly wrap over the buffer end when it is reached",
        should_wrap_on_buffer_limit
    );
    return NULL;
}

#undef BUFFER_SIZE_LOG2N
#undef BUFFER_SIZE
