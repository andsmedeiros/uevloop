#include "event.h"

/// \cond
#include <stdlib.h>
/// \endcond

void uel_event_destroy(uel_event_t *event){
    uel_closure_destroy(&event->closure);
}

void uel_event_config_closure(uel_event_t *event, uel_closure_t *closure, bool repeating){
    event->type = CLOSURE_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
}

void uel_event_config_signal(
    uel_event_t *event,
    uintptr_t signal,
    uel_llist_t *listeners,
    void *params
){
    event->closure = uel_closure_create(NULL, NULL, NULL);
    event->type = SIGNAL_EVENT;
    event->detail.signal.value = signal;
    event->detail.signal.listeners = listeners;
    event->closure.params = params;
}

void uel_event_config_signal_listener(uel_event_t *event, uel_closure_t *closure, bool repeating){
    event->type = SIGNAL_LISTENER_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
    event->detail.listener.unlistened = false;
}

void uel_event_config_timer(
    uel_event_t *event,
    uint16_t timeout_in_ms,
    bool repeating,
    bool immediate,
    uel_closure_t *closure,
    uint32_t current_time
){
    event->type = TIMER_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
    event->detail.timer.due_time = immediate ?
        current_time :
        current_time + timeout_in_ms;
    event->detail.timer.timeout = timeout_in_ms;
}
