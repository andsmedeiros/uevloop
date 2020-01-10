#include "event-loop.h"

#include <stdlib.h>

#include "system/event-loop.h"
#include "system/pools.h"
#include "utils/circular-queue.h"
#include "utils/closure.h"
#include "../minunit.h"

#define DECLARE_EVENT_LOOP()                                                    \
    pools_t pools;                                                              \
    void *event_queue_buffer[8];                                                \
    cqueue_t event_queue;                                                       \
    void *reschedule_queue_buffer[8];                                           \
    cqueue_t reschedule_queue;                                                  \
    pools_init(&pools);                                                         \
    cqueue_init(&event_queue, event_queue_buffer, 3);                           \
    cqueue_init(&reschedule_queue, reschedule_queue_buffer, 3 );                \
    evloop_t loop;                                                              \
    evloop_init(&loop, &pools.event_pool, &event_queue, &reschedule_queue);

static char *should_init_event_loop(){
    DECLARE_EVENT_LOOP();

    mu_assert_pointers_equal("evloop.event_pool", &pools.event_pool, loop.event_pool);
    mu_assert_pointers_equal("evloop.event_queue", &event_queue, loop.event_queue);
    mu_assert_pointers_equal(
        "evloop.reschedule_queue",
        &reschedule_queue,
        loop.reschedule_queue
    );

    return NULL;
}

static void *nop(closure_t *closure){ return NULL; }
static char *should_enqueue_events(){
    DECLARE_EVENT_LOOP();

    closure_t closure = closure_create(&nop, NULL, NULL);
    evloop_enqueue_closure(&loop, &closure);

    mu_assert_ints_equal("event_queue.count after first insertion", 1, event_queue.count);

    event_t timer;
    event_config_timer(&timer, 1000, false, false, &closure, 0);
    evloop_enqueue_event(&loop, &timer);

    mu_assert_ints_equal("event_queue.count after second insertion", 2, event_queue.count);

    return NULL;
}

static void *mark_execution(closure_t *closure){
    bool *executed = (bool *)closure->context;
    *executed = true;

    return NULL;
}
static char *should_run_events(){
    DECLARE_EVENT_LOOP();

    bool done1 = false;
    closure_t closure1 = closure_create(&mark_execution, (void *)&done1, NULL);
    evloop_enqueue_closure(&loop, &closure1);

    bool done2 = false;
    closure_t closure2 = closure_create(&mark_execution, (void *)&done2, NULL);
    event_t timer;
    event_config_timer(&timer, 1000, false, false, &closure2, 0);
    evloop_enqueue_event(&loop, &timer);

    evloop_run(&loop);

    mu_assert("done1 must had been set", done1);
    mu_assert("done2 must had been set", done2);
    mu_assert_int_zero("event_queue.count", event_queue.count);

    return NULL;
}

static char *should_reschedule_expired_timers(){
    DECLARE_EVENT_LOOP();

    closure_t closure = closure_create(&nop, NULL, NULL);
    event_t timer;
    event_config_timer(&timer, 100, true, false, &closure, 0);
    evloop_enqueue_event(&loop, &timer);
    evloop_run(&loop);

    mu_assert_ints_equal("reschedule_queue.count", 1, reschedule_queue.count);

    return NULL;
}

char *evloop_run_tests(){
    mu_run_test(
        "should correctly initialise an event loop",
        should_init_event_loop
    );
    mu_run_test(
        "should correctly enqueue event and closures",
        should_enqueue_events
    );
    mu_run_test(
        "should correctly run enqueued event and closures",
        should_run_events
    );
    mu_run_test(
        "should correctly make run timers available for rescheduling if they " \
            "are repeating",
        should_reschedule_expired_timers
    );

    return NULL;
}
