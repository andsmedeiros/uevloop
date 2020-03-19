#include "event.h"

#include <stdlib.h>
#include <stdint.h>

#include "system/event.h"
#include "../uelt.h"

static void *nop(uel_closure_t *closure){ return NULL; }

static char *should_config_uel_closure_event(){
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);

    uel_event_config_closure(&event, &closure, false);

    uelt_assert_ints_equal("event.type", CLOSURE_EVENT, event.type);
    uelt_assert_pointers_equal(
        "event.closure.function",
        &nop,
        event.closure.function
    );
    uelt_assert_not("event.repeating", event.repeating);

    uel_event_config_closure(&event, &closure, true);
    uelt_assert("event.repeating", event.repeating);

    return NULL;
}

static char *should_config_timer_event(){
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);

    uint32_t timer = 326680;
    uint16_t timeout_in_ms = 15000;

    uel_event_config_timer(&event, timeout_in_ms, true, false, &closure, timer);
    uelt_assert_ints_equal("event.type", TIMER_EVENT, event.type);
    uelt_assert_pointers_equal(
        "event.closure.function",
        &nop,
        event.closure.function
    );
    uelt_assert_ints_equal(
        "event.detail.timer.timeout",
        timeout_in_ms,
        event.detail.timer.timeout
    );
    uelt_assert("event.repeating muts had been set", event.repeating);
    uelt_assert_ints_equal(
        "event.detail.timer.due_time",
        timer + timeout_in_ms,
        event.detail.timer.due_time
    );

    return NULL;
}

typedef enum uel_event_test_signals {
    SIGNAL_0, SIGNAL_1, SIGNAL_MAX
 } uel_event_test_signals_t;

static char *should_config_signal_event(){
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);
    uel_llist_t listeners[SIGNAL_MAX];

    for (size_t i = 0; i < SIGNAL_MAX; i++) {
        uel_llist_init(&listeners[i]);
    }
    uel_event_config_signal(&event, SIGNAL_0, listeners, (void *)&closure);
    uelt_assert_ints_equal("event.type", SIGNAL_EVENT, event.type);
    uelt_assert_ints_equal(
        "event.detail.signal.value",
        SIGNAL_0,
        event.detail.signal.value
    );
    uelt_assert_pointers_equal(
        "event.detail.signal.listeners",
        &listeners,
        event.detail.signal.listeners
    );
    uelt_assert_pointers_equal(
        "event.closure.params",
        &closure,
        event.closure.params
    );

    return NULL;
}

static void uel_signal_destruction(uel_closure_t *closure){
    bool *destroyed = (bool *)closure->context;
    *destroyed = true;
}
static char *should_destroy_events(){
    {
        uel_event_t event;
        bool destroyed = false;

        uel_closure_t closure = uel_closure_create(&nop, &destroyed, uel_signal_destruction);
        uel_event_config_closure(&event, &closure, false);
        uel_event_destroy(&event);

        uelt_assert("destroyed must had been set on closure event destruction", destroyed);
    }
    {
        uel_event_t event;
        bool destroyed = false;

        uel_closure_t closure = uel_closure_create(&nop, &destroyed, uel_signal_destruction);
        uel_event_config_timer(&event, 1000, false, true, &closure, 10000);
        uel_event_destroy(&event);

        uelt_assert("destroyed must had been set on timer event destruction", destroyed);
    }

    return NULL;
}

char *event_run_tests(){
    uelt_run_test(
        "should correctly config a closure event",
        should_config_uel_closure_event
    );
    uelt_run_test(
        "should correctly config a timer event",
        should_config_timer_event
    );
    uelt_run_test(
        "should correctly config a signal event",
        should_config_signal_event
    );
    uelt_run_test(
        "should correctly destroy events and contained closures",
        should_destroy_events
    );

    return NULL;
}
