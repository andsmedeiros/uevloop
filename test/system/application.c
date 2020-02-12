#include "application.h"

#include <stdlib.h>
#include "system/application.h"
#include "../minunit.h"

#define DECLARE_APP()                                                           \
    application_t app;                                                          \
    app_init(&app);

static char *should_init_app(){
    DECLARE_APP();

    mu_assert_pointers_equal(
        "app.pools.event_pool.buffer",
        app.pools.event_pool_buffer,
        app.pools.event_pool.buffer
    );
    mu_assert_pointers_equal(
        "app.pools.llist_node_pool.buffer",
        app.pools.llist_node_pool_buffer,
        app.pools.llist_node_pool.buffer
    );
    mu_assert_pointers_equal(
        "app.queues.event_queue.buffer",
        app.queues.event_queue_buffer,
        app.queues.event_queue.buffer
    );
    mu_assert_pointers_equal(
        "app.queues.schedule_queue.buffer",
        app.queues.schedule_queue_buffer,
        app.queues.schedule_queue.buffer
    );
    mu_assert_pointers_equal(
        "app.scheduler.pools",
        &app.pools,
        app.scheduler.pools
    );
    mu_assert_pointers_equal(
        "app.scheduler.queues",
        &app.queues,
        app.scheduler.queues
    );
    mu_assert_pointers_equal(
        "app.event_loop.pools",
        &app.pools,
        app.event_loop.pools
    );
    mu_assert_pointers_equal(
        "app.event_loop.queues",
        &app.queues,
        app.event_loop.queues
    );
    mu_assert_pointers_equal(
        "app.relay.queues",
        &app.queues,
        app.relay.queues
    );
    mu_assert_pointers_equal(
        "app.relay.pools",
        &app.pools,
        app.relay.pools
    );
    mu_assert_pointers_equal(
        "app.relay.signal_vector",
        app.relay_buffer,
        app.relay.signal_vector
    );
    mu_assert_ints_equal("app.relay.width", APP_EVENT_COUNT, app.relay.width);
    mu_assert("app.run_scheduler must had been set", app.run_scheduler);
    return NULL;
}

static char *should_update_timer(){
    DECLARE_APP();

    mu_assert_int_zero("app.scheduler.timer", app.scheduler.timer);

    app_update_timer(&app, 10);
    mu_assert_ints_equal(
        "app.scheduler.timer after first set",
        10,
        app.scheduler.timer
    );

    app_update_timer(&app, 100);
    mu_assert_ints_equal(
        "app.scheduler.timer after first set",
        100,
        app.scheduler.timer
    );

    return NULL;
}

static char *should_set_scheduler_run_flag(){
    DECLARE_APP();

    app_tick(&app);
    mu_assert_not("app.run_scheduler must had been unset", app.run_scheduler);

    app_update_timer(&app, 100);
    mu_assert("app.run_scheduler must had been set", app.run_scheduler);

    app_tick(&app);
    mu_assert_not("app.run_scheduler must had been unset", app.run_scheduler);

    app_tick(&app);
    mu_assert_not("app.run_scheduler must had been unset", app.run_scheduler);

    return NULL;
}

static void *increment(closure_t *closure){
    uintptr_t *counter = (uintptr_t *)closure->context;
    (*counter)++;

    return NULL;
}
static char *should_tick(){
    DECLARE_APP();

    uintptr_t counter1 = 0, counter2 = 0, counter3 = 0;

    closure_t closure1 = closure_create(&increment, (void *)&counter1, NULL);
    closure_t closure2 = closure_create(&increment, (void *)&counter2, NULL);
    closure_t closure3 = closure_create(&increment, (void *)&counter3, NULL);

    evloop_enqueue_closure(&app.event_loop, &closure1);
    sch_run_later(&app.scheduler, 100, closure2);
    sch_run_at_intervals(&app.scheduler, 100, true, closure3);

    app_tick(&app);
    mu_assert_ints_equal("counter1 at 0ms", 1, counter1);
    mu_assert_int_zero("counter2 at 0ms", counter2);
    mu_assert_ints_equal("counter3 at 0ms", 1, counter3);

    app_update_timer(&app, 50);
    app_tick(&app);
    mu_assert_ints_equal("counter1 at 50ms", 1, counter1);
    mu_assert_int_zero("counter2 at 50ms", counter2);
    mu_assert_ints_equal("counter3 at 50ms", 1, counter3);

    app_update_timer(&app, 100);
    app_tick(&app);
    mu_assert_ints_equal("counter1 at 100ms", 1, counter1);
    mu_assert_ints_equal("counter2 at 100ms", 1, counter2);
    mu_assert_ints_equal("counter3 at 100ms", 2, counter3);

    return NULL;
}

static void *nop(closure_t *closure){
    return NULL;
}
static char *should_proxy_functions(){
    DECLARE_APP();

    closure_t closure = closure_create(&nop, NULL, NULL);

    app_enqueue_closure(&app, &closure);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        1,
        sysqueues_count_enqueued_events(&app.queues)
    );
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        0,
        sysqueues_count_scheduled_events(&app.queues)
    );

    app_run_later(&app, 1000, closure);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        1,
        sysqueues_count_enqueued_events(&app.queues)
    );
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        1,
        sysqueues_count_scheduled_events(&app.queues)
    );

    app_run_at_intervals(&app, 500, false, closure);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        1,
        sysqueues_count_enqueued_events(&app.queues)
    );
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        2,
        sysqueues_count_scheduled_events(&app.queues)
    );

    app_run_at_intervals(&app, 500, true, closure);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        2,
        sysqueues_count_enqueued_events(&app.queues)
    );
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        2,
        sysqueues_count_scheduled_events(&app.queues)
    );

    return NULL;
}

char *app_run_tests(){

    mu_run_test("should correctly initialise an application", should_init_app);
    mu_run_test(
        "should correctly update an application internal timer",
        should_update_timer
    );
    mu_run_test(
        "should correctly set the scheduler run flag when the timer is updated",
        should_set_scheduler_run_flag
    );
    mu_run_test(
        "should correctly tick an application event loop and operate accordingly",
        should_tick
    );
    mu_run_test(
        "should correctly proxy scheduler and event loop functions",
        should_proxy_functions
    );

    return NULL;
}
