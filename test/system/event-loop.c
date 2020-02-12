#include "event-loop.h"

#include <stdlib.h>

#include "system/event-loop.h"
#include "system/containers/system-pools.h"
#include "utils/circular-queue.h"
#include "utils/closure.h"
#include "../minunit.h"

#define DECLARE_EVENT_LOOP()                                                    \
    syspools_t pools;                                                              \
    syspools_init(&pools);                                                         \
    sysqueues_t queues;                                                         \
    sysqueues_init(&queues);                                                    \
    evloop_t loop;                                                              \
    evloop_init(&loop, &pools, &queues);

static char *should_init_event_loop(){
    DECLARE_EVENT_LOOP();

    mu_assert_pointers_equal("evloop.system_pools", &pools, loop.pools);
    mu_assert_pointers_equal("evloop.queues", &queues, loop.queues);

    return NULL;
}

static void *nop(closure_t *closure){ return NULL; }
static char *should_enqueue_closures(){
    DECLARE_EVENT_LOOP();

    closure_t closure = closure_create(&nop, NULL, NULL);
    evloop_enqueue_closure(&loop, &closure);

    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events after first insertion",
        1,
        sysqueues_count_enqueued_events(&queues)
    );
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
    evloop_enqueue_closure(&loop, &closure2);

    evloop_run(&loop);

    mu_assert("done1 must had been set", done1);
    mu_assert("done2 must had been set", done2);
    mu_assert_int_zero(
        "sysqueues_count_enqueued_events",
        sysqueues_count_enqueued_events(&queues)
    );

    return NULL;
}

static char *should_schedule_expired_timers(){
    DECLARE_EVENT_LOOP();

    closure_t closure = closure_create(&nop, NULL, NULL);
    event_t timer;
    event_config_timer(&timer, 100, true, false, &closure, 0);
    sysqueues_enqueue_event(loop.queues, &timer);
    evloop_run(&loop);

    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        1,
        sysqueues_count_scheduled_events(&queues)
    );

    return NULL;
}

char *evloop_run_tests(){
    mu_run_test(
        "should correctly initialise an event loop",
        should_init_event_loop
    );
    mu_run_test(
        "should correctly enqueue event and closures",
        should_enqueue_closures
    );
    mu_run_test(
        "should correctly run enqueued event and closures",
        should_run_events
    );
    mu_run_test(
        "should correctly make run timers available for rescheduling if they " \
            "are repeating",
        should_schedule_expired_timers
    );

    return NULL;
}
