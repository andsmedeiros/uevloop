#include "event-loop.h"

#include <stdlib.h>

#include "system/event-loop.h"
#include "system/containers/system-pools.h"
#include "utils/circular-queue.h"
#include "utils/closure.h"
#include "../uelt.h"

#define DECLARE_EVENT_LOOP()                                                    \
    uel_syspools_t pools;                                                              \
    uel_syspools_init(&pools);                                                         \
    uel_sysqueues_t queues;                                                         \
    uel_sysqueues_init(&queues);                                                    \
    uel_evloop_t loop;                                                              \
    uel_evloop_init(&loop, &pools, &queues);

static char *should_init_event_loop(){
    DECLARE_EVENT_LOOP();

    uelt_assert_pointers_equal("evloop.system_pools", &pools, loop.pools);
    uelt_assert_pointers_equal("evloop.queues", &queues, loop.queues);

    return NULL;
}

static void *nop(uel_closure_t *closure){ return NULL; }
static char *should_enqueue_closures(){
    DECLARE_EVENT_LOOP();

    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);
    uel_evloop_enqueue_closure(&loop, &closure);

    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events after first insertion",
        1,
        uel_sysqueues_count_enqueued_events(&queues)
    );
    return NULL;
}

static void *mark_execution(uel_closure_t *closure){
    bool *executed = (bool *)closure->context;
    *executed = true;

    return NULL;
}
static char *should_run_events(){
    DECLARE_EVENT_LOOP();

    bool done1 = false;
    uel_closure_t closure1 = uel_closure_create(&mark_execution, (void *)&done1, NULL);
    uel_evloop_enqueue_closure(&loop, &closure1);

    bool done2 = false;
    uel_closure_t closure2 = uel_closure_create(&mark_execution, (void *)&done2, NULL);
    uel_evloop_enqueue_closure(&loop, &closure2);

    uel_evloop_run(&loop);

    uelt_assert("done1 must had been set", done1);
    uelt_assert("done2 must had been set", done2);
    uelt_assert_int_zero(
        "uel_sysqueues_count_enqueued_events",
        uel_sysqueues_count_enqueued_events(&queues)
    );

    return NULL;
}

static char *should_schedule_expired_timers(){
    DECLARE_EVENT_LOOP();

    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);
    uel_event_t *timer = uel_syspools_acquire_event(&pools);
    uel_event_config_timer(timer, 100, true, false, &closure, 0);
    uel_sysqueues_enqueue_event(loop.queues, timer);
    uel_evloop_run(&loop);

    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(&queues)
    );

    return NULL;
}

static char *should_handle_paused_and_cancelled_timers(){
    DECLARE_EVENT_LOOP();
    uint32_t counter = 0;

    bool flag = false;
    uel_closure_t closure = uel_closure_create(&mark_execution, (void *)&flag, NULL);

    uel_event_t *timer = uel_syspools_acquire_event(loop.pools);
    uel_event_config_timer(timer, 10, true, false, &closure, counter);
    uel_sysqueues_enqueue_event(loop.queues, timer);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(loop.queues)
    );

    uel_event_timer_pause(timer);
    uel_evloop_run(&loop);
    uelt_assert_int_zero(
        "uel_sysqueues_count_enqueued_events",
        uel_sysqueues_count_enqueued_events(loop.queues)
    );
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(loop.queues)
    );
    uelt_assert_not("flag", flag);

    timer = uel_syspools_acquire_event(loop.pools);
    uel_event_config_timer(timer, 10, true, false, &closure, counter);
    uel_sysqueues_enqueue_event(loop.queues, timer);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(loop.queues)
    );

    uel_event_timer_cancel(timer);
    uel_evloop_run(&loop);
    uelt_assert_int_zero(
        "uel_sysqueues_count_enqueued_events",
        uel_sysqueues_count_enqueued_events(loop.queues)
    );
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(loop.queues)
    );
    uelt_assert_not("flag", flag);
    uelt_assert_ints_equal(
        "pools.event_pool.queue.count",
        UEL_SYSPOOLS_EVENT_POOL_SIZE - 1,
        pools.event_pool.queue.count
    );

    return NULL;
}

char *uel_evloop_run_tests(){
    uelt_run_test(
        "should correctly initialise an event loop",
        should_init_event_loop
    );
    uelt_run_test(
        "should correctly enqueue event and closures",
        should_enqueue_closures
    );
    uelt_run_test(
        "should correctly run enqueued event and closures",
        should_run_events
    );
    uelt_run_test(
        "should correctly make timers available for rescheduling if they " \
            "are repeating",
        should_schedule_expired_timers
    );
    uelt_run_test(
        "should correctly handle paused and cancelled timers",
        should_handle_paused_and_cancelled_timers
    );

    return NULL;
}
