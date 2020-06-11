#include "event.h"

#include <stdlib.h>
#include <stdint.h>

#include "uevloop/system/event.h"
#include "../uelt.h"

static void *nop(void *context, void *params){ return NULL; }

static char *should_config_uel_closure_event(){
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(&nop, NULL);

    uel_event_config_closure(&event, &closure, false);

    uelt_assert_ints_equal("event.type", UEL_CLOSURE_EVENT, event.type);
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
    uel_closure_t closure = uel_closure_create(&nop, NULL);

    uint32_t timer = 326680;
    uint16_t timeout_in_ms = 15000;

    uel_event_config_timer(&event, timeout_in_ms, true, false, &closure, timer);
    uelt_assert_ints_equal("event.type", UEL_TIMER_EVENT, event.type);
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
    uelt_assert("event.repeating", event.repeating);
    uelt_assert_ints_equal(
        "event.detail.timer.due_time",
        timer + timeout_in_ms,
        event.detail.timer.due_time
    );
    uelt_assert_ints_equal(
        "event.detail.timer.status",
        UEL_TIMER_RUNNING,
        event.detail.timer.status
    );

    uel_event_timer_pause(&event);
    uelt_assert_ints_equal(
        "event.detail.timer.status",
        UEL_TIMER_PAUSED,
        event.detail.timer.status
    );

    uel_event_timer_resume(&event);
    uelt_assert_ints_equal(
        "event.detail.timer.status",
        UEL_TIMER_RUNNING,
        event.detail.timer.status
    );

    uel_event_timer_cancel(&event);
    uelt_assert_ints_equal(
        "event.detail.timer.status",
        UEL_TIMER_CANCELLED,
        event.detail.timer.status
    );

    return NULL;
}

typedef enum uel_event_test_signals {
    SIGNAL_0, SIGNAL_1, SIGNAL_MAX
 } uel_event_test_signals_t;

static char *should_config_signal_event(){
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(&nop, NULL);
    uel_llist_t listeners[SIGNAL_MAX];

    for (size_t i = 0; i < SIGNAL_MAX; i++) {
        uel_llist_init(&listeners[i]);
    }
    uel_event_config_signal(&event, SIGNAL_0, listeners, (void *)&closure);
    uelt_assert_ints_equal("event.type", UEL_SIGNAL_EVENT, event.type);
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
    uelt_assert_pointers_equal("event.value", &closure, event.value);

    return NULL;
}

static char *should_config_signal_listener_event(){
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(&nop, NULL);

    uel_event_config_signal_listener(&event, &closure, true);
    uelt_assert_ints_equal("event.type", UEL_SIGNAL_LISTENER_EVENT, event.type);
    uelt_assert("event.repeating", event.repeating);
    uelt_assert_not("event.detail.listener.unlistened", event.detail.listener.unlistened);

    return NULL;
}

static char *should_config_observer_event(){
    volatile uintptr_t cv = 10;
    uel_event_t event;
    uel_closure_t closure = uel_closure_create(nop, NULL);

    uel_event_config_observer(&event, &closure, &cv, true);
    uelt_assert_ints_equal("event.type", UEL_OBSERVER_EVENT, event.type);
    uelt_assert("event.repeating", event.repeating);
    uelt_assert_pointers_equal(
        "event.detail.observer.condition_var",
        &cv,
        event.detail.observer.condition_var
    );
    uelt_assert_ints_equal(
        "event.detail.observer.last_value",
        cv,
        event.detail.observer.last_value
    );
    uelt_assert_not(
        "event.detail.observer.cancelled",
        event.detail.observer.cancelled
    );

    uel_event_observer_cancel(&event);
    uelt_assert(
        "event.detail.observer.cancelled",
        event.detail.observer.cancelled
    );

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
        "should correctly config a signal listener event",
        should_config_signal_listener_event
    );
    uelt_run_test(
        "should correctly config an observer event",
        should_config_observer_event
    );

    return NULL;
}
