#include "event.h"

/// \cond
#include <stdlib.h>
/// \endcond

void uel_event_destroy(uel_event_t *event){
    uel_closure_destroy(&event->closure);
}

void uel_event_config_closure(uel_event_t *event, uel_closure_t *closure, bool repeating){
    event->type = UEL_CLOSURE_EVENT;
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
    event->type = UEL_SIGNAL_EVENT;
    event->detail.signal.value = signal;
    event->detail.signal.listeners = listeners;
    event->closure.params = params;
}

void uel_event_config_signal_listener(uel_event_t *event, uel_closure_t *closure, bool repeating){
    event->type = UEL_SIGNAL_LISTENER_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
    event->detail.listener.unlistened = false;
}

void uel_event_config_observer(
    uel_event_t *event,
    uel_closure_t *closure,
    volatile uintptr_t *condition_var,
    bool repeating
){
    event->type = UEL_OBSERVER_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
    event->detail.observer.last_value = *condition_var;
    event->detail.observer.condition_var = condition_var;
}

void uel_event_observer_cancel(uel_event_t *event){
    event->detail.observer.cancelled = true;
}

void uel_event_config_timer(
    uel_event_t *event,
    uint16_t timeout_in_ms,
    bool repeating,
    bool immediate,
    uel_closure_t *closure,
    uint32_t current_time
){
    event->type = UEL_TIMER_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
    event->detail.timer.due_time = immediate ?
        current_time :
        current_time + timeout_in_ms;
    event->detail.timer.timeout = timeout_in_ms;
    event->detail.timer.status = UEL_TIMER_RUNNING;
}

void uel_event_timer_pause(uel_event_t *event){
    event->detail.timer.status = UEL_TIMER_PAUSED;
}

void uel_event_timer_resume(uel_event_t *event){
    event->detail.timer.status = UEL_TIMER_RUNNING;
}

void uel_event_timer_cancel(uel_event_t *event){
    event->detail.timer.status = UEL_TIMER_CANCELLED;
}
