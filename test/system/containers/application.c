#include "application.h"

#include <stdlib.h>
#include "system/containers/application.h"
#include "utils/module.h"
#include "test/uelt.h"

enum TEST_APP_MODULES {
    TEST_APP_MOD0,
    TEST_APP_MOD1,
    TEST_APP_MOD_COUNT
};

#define DECLARE_APP()                               \
    uel_application_t app;                          \
    uel_app_init(&app);

static char *should_init_app(){
    DECLARE_APP();

    uelt_assert_pointer_null("app.registry", app.registry);
    uelt_assert_int_zero("app.registry_size", app.registry_size);
    uelt_assert_pointers_equal(
        "app.pools.event_pool.buffer",
        app.pools.event_pool_buffer,
        app.pools.event_pool.buffer
    );
    uelt_assert_pointers_equal(
        "app.pools.llist_node_pool.buffer",
        app.pools.llist_node_pool_buffer,
        app.pools.llist_node_pool.buffer
    );
    uelt_assert_pointers_equal(
        "app.queues.event_queue.buffer",
        app.queues.event_queue_buffer,
        app.queues.event_queue.buffer
    );
    uelt_assert_pointers_equal(
        "app.queues.schedule_queue.buffer",
        app.queues.schedule_queue_buffer,
        app.queues.schedule_queue.buffer
    );
    uelt_assert_pointers_equal(
        "app.scheduler.pools",
        &app.pools,
        app.scheduler.pools
    );
    uelt_assert_pointers_equal(
        "app.scheduler.queues",
        &app.queues,
        app.scheduler.queues
    );
    uelt_assert_pointers_equal(
        "app.event_loop.pools",
        &app.pools,
        app.event_loop.pools
    );
    uelt_assert_pointers_equal(
        "app.event_loop.queues",
        &app.queues,
        app.event_loop.queues
    );
    uelt_assert_pointers_equal(
        "app.relay.queues",
        &app.queues,
        app.relay.queues
    );
    uelt_assert_pointers_equal(
        "app.relay.pools",
        &app.pools,
        app.relay.pools
    );
    uelt_assert_pointers_equal(
        "app.relay.signal_vector",
        app.relay_buffer,
        app.relay.signal_vector
    );
    uelt_assert_ints_equal("app.relay.width", UEL_APP_EVENT_COUNT, app.relay.width);
    uelt_assert("app.run_scheduler must had been set", app.run_scheduler);
    return NULL;
}


struct test_module {
    uel_module_t base;
    unsigned int configs;
    unsigned int launches;
};

static void config(uel_module_t *mod){ ((struct test_module *)mod)->configs++; }
static void launch(uel_module_t *mod){ ((struct test_module *)mod)->launches++; }

static char *should_handle_modules(){
    DECLARE_APP();

    struct test_module module0 = { .configs = 0, .launches = 0 };
    struct test_module module1 = { .configs = 0, .launches = 0 };
    uel_module_init(&module0.base, config, launch, &app);
    uel_module_init(&module1.base, config, launch, &app);
    uel_module_t *modules[TEST_APP_MOD_COUNT];
    modules[TEST_APP_MOD0] = &module0.base;
    modules[TEST_APP_MOD1] = &module1.base;

    uel_app_boot(&app, modules, TEST_APP_MOD_COUNT);
    uelt_assert_pointers_equal("app.registry", modules, app.registry);
    uelt_assert_ints_equal("app.registry_size", TEST_APP_MOD_COUNT, app.registry_size);
    uelt_assert_ints_equal("module0.configs", 1, module0.configs);
    uelt_assert_ints_equal("module0.launches", 1, module0.launches);
    uelt_assert_ints_equal("module1.configs", 1, module1.configs);
    uelt_assert_ints_equal("module1.launches", 1, module1.launches);

    uel_module_t *mod0 = uel_app_require(&app, TEST_APP_MOD0);
    uelt_assert_pointers_equal("mod0", &module0, mod0);
    uel_module_t *mod1 = uel_app_require(&app, TEST_APP_MOD1);
    uelt_assert_pointers_equal("mod1", &module1, mod1);

    return NULL;
}

static char *should_update_timer(){
    DECLARE_APP();

    uelt_assert_int_zero("app.scheduler.timer", app.scheduler.timer);

    uel_app_update_timer(&app, 10);
    uelt_assert_ints_equal(
        "app.scheduler.timer after first set",
        10,
        app.scheduler.timer
    );

    uel_app_update_timer(&app, 100);
    uelt_assert_ints_equal(
        "app.scheduler.timer after first set",
        100,
        app.scheduler.timer
    );

    return NULL;
}

static char *should_set_uel_scheduer_run_flag(){
    DECLARE_APP();

    uel_app_tick(&app);
    uelt_assert_not("app.run_scheduler must had been unset", app.run_scheduler);

    uel_app_update_timer(&app, 100);
    uelt_assert("app.run_scheduler must had been set", app.run_scheduler);

    uel_app_tick(&app);
    uelt_assert_not("app.run_scheduler must had been unset", app.run_scheduler);

    uel_app_tick(&app);
    uelt_assert_not("app.run_scheduler must had been unset", app.run_scheduler);

    return NULL;
}

static void *increment(uel_closure_t *closure){
    uintptr_t *counter = (uintptr_t *)closure->context;
    (*counter)++;

    return NULL;
}
static char *should_tick(){
    DECLARE_APP();

    uintptr_t counter1 = 0, counter2 = 0, counter3 = 0;

    uel_closure_t closure1 = uel_closure_create(&increment, (void *)&counter1, NULL);
    uel_closure_t closure2 = uel_closure_create(&increment, (void *)&counter2, NULL);
    uel_closure_t closure3 = uel_closure_create(&increment, (void *)&counter3, NULL);

    uel_evloop_enqueue_closure(&app.event_loop, &closure1);
    uel_sch_run_later(&app.scheduler, 100, closure2);
    uel_sch_run_at_intervals(&app.scheduler, 100, true, closure3);

    uel_app_tick(&app);
    uelt_assert_ints_equal("counter1 at 0ms", 1, counter1);
    uelt_assert_int_zero("counter2 at 0ms", counter2);
    uelt_assert_ints_equal("counter3 at 0ms", 1, counter3);

    uel_app_update_timer(&app, 50);
    uel_app_tick(&app);
    uelt_assert_ints_equal("counter1 at 50ms", 1, counter1);
    uelt_assert_int_zero("counter2 at 50ms", counter2);
    uelt_assert_ints_equal("counter3 at 50ms", 1, counter3);

    uel_app_update_timer(&app, 100);
    uel_app_tick(&app);
    uelt_assert_ints_equal("counter1 at 100ms", 1, counter1);
    uelt_assert_ints_equal("counter2 at 100ms", 1, counter2);
    uelt_assert_ints_equal("counter3 at 100ms", 2, counter3);

    return NULL;
}

static void *nop(uel_closure_t *closure){
    return NULL;
}
static char *should_proxy_functions(){
    DECLARE_APP();

    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);

    uel_app_enqueue_closure(&app, &closure);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(&app.queues)
    );
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        0,
        uel_sysqueues_count_scheduled_events(&app.queues)
    );

    uel_app_run_later(&app, 1000, closure);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(&app.queues)
    );
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(&app.queues)
    );

    uel_app_run_at_intervals(&app, 500, false, closure);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(&app.queues)
    );
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        2,
        uel_sysqueues_count_scheduled_events(&app.queues)
    );

    uel_app_run_at_intervals(&app, 500, true, closure);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        2,
        uel_sysqueues_count_enqueued_events(&app.queues)
    );
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        2,
        uel_sysqueues_count_scheduled_events(&app.queues)
    );

    volatile uintptr_t counter = 0;
    uel_event_t *observer = uel_app_observe(&app, &counter, &closure);
    uelt_assert_ints_equal("app.event_loop.observers.count", 1, app.event_loop.observers.count);
    uelt_assert_pointer_not_null("observer", observer);
    uelt_assert_pointers_equal(observer->detail.observer.condition_var, &counter, observer->detail.observer.condition_var);

    return NULL;
}

char *uel_app_run_tests(){

    uelt_run_test("should correctly initialise an application", should_init_app);
    uelt_run_test("should correctly handle modules", should_handle_modules);
    uelt_run_test(
        "should correctly update an application internal timer",
        should_update_timer
    );
    uelt_run_test(
        "should correctly set the scheduler run flag when the timer is updated",
        should_set_uel_scheduer_run_flag
    );
    uelt_run_test(
        "should correctly tick an application event loop and operate accordingly",
        should_tick
    );
    uelt_run_test(
        "should correctly proxy scheduler and event loop functions",
        should_proxy_functions
    );

    return NULL;
}
