#include "event.h"

#include <stdlib.h>
#include <stdint.h>

#include "system/event.h"
#include "../minunit.h"

static void *nop(closure_t *closure){ return NULL; }

static char *should_config_closure_event(){
    event_t event;
    closure_t closure = closure_create(&nop, NULL, NULL);

    event_config_closure(&event, &closure, false);

    mu_assert_ints_equal("event.type", CLOSURE_EVENT, event.type);
    mu_assert_pointers_equal(
        "event.closure.function",
        &nop,
        event.closure.function
    );
    mu_assert_not("event.repeating", event.repeating);

    event_config_closure(&event, &closure, true);
    mu_assert("event.repeating", event.repeating);

    return NULL;
}

static char *should_config_timer_event(){
    event_t event;
    closure_t closure = closure_create(&nop, NULL, NULL);

    uint32_t timer = 326680;
    uint16_t timeout_in_ms = 15000;

    event_config_timer(&event, timeout_in_ms, true, false, &closure, timer);
    mu_assert_ints_equal("event.type", TIMER_EVENT, event.type);
    mu_assert_pointers_equal(
        "event.closure.function",
        &nop,
        event.closure.function
    );
    mu_assert_ints_equal(
        "event.detail.timer.timeout",
        timeout_in_ms,
        event.detail.timer.timeout
    );
    mu_assert("event.repeating muts had been set", event.repeating);
    mu_assert_ints_equal(
        "event.detail.timer.due_time",
        timer + timeout_in_ms,
        event.detail.timer.due_time
    );

    return NULL;
}

typedef enum event_test_signals {
    SIGNAL_0, SIGNAL_1, SIGNAL_MAX
 } event_test_signals_t;

static char *should_config_signal_event(){
    event_t event;
    closure_t closure = closure_create(&nop, NULL, NULL);
    llist_t listeners[SIGNAL_MAX];

    for (size_t i = 0; i < SIGNAL_MAX; i++) {
        llist_init(&listeners[i]);
    }
    event_config_signal(&event, SIGNAL_0, listeners, (void *)&closure);
    mu_assert_ints_equal("event.type", SIGNAL_EVENT, event.type);
    mu_assert_ints_equal(
        "event.detail.signal.value",
        SIGNAL_0,
        event.detail.signal.value
    );
    mu_assert_pointers_equal(
        "event.detail.signal.listeners",
        &listeners,
        event.detail.signal.listeners
    );
    mu_assert_pointers_equal(
        "event.closure.params",
        &closure,
        event.closure.params
    );

    return NULL;
}

static void signal_destruction(closure_t *closure){
    bool *destroyed = (bool *)closure->context;
    *destroyed = true;
}
static char *should_destroy_events(){
    {
        event_t event;
        bool destroyed = false;

        closure_t closure = closure_create(&nop, &destroyed, &signal_destruction);
        event_config_closure(&event, &closure, false);
        event_destroy(&event);

        mu_assert("destroyed must had been set on closure event destruction", destroyed);
    }
    {
        event_t event;
        bool destroyed = false;

        closure_t closure = closure_create(&nop, &destroyed, &signal_destruction);
        event_config_timer(&event, 1000, false, true, &closure, 10000);
        event_destroy(&event);

        mu_assert("destroyed must had been set on timer event destruction", destroyed);
    }

    return NULL;
}

char *event_run_tests(){
    mu_run_test(
        "should correctly config a closure event",
        should_config_closure_event
    );
    mu_run_test(
        "should correctly config a timer event",
        should_config_timer_event
    );
    mu_run_test(
        "should correctly config a signal event",
        should_config_signal_event
    );
    mu_run_test(
        "should correctly destroy events and contained closures",
        should_destroy_events
    );

    return NULL;
}
