#include "event.h"

void event_destroy(event_t *event){
    closure_destroy(&event->closure);
}

void event_config_closure(event_t *event, closure_t *closure){
    event->type = CLOSURE_EVENT;
    event->closure = *closure;
}

void event_config_signal(event_t *event, bool repeating, closure_t *closure){
    event->type = SIGNAL_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
}

void event_config_timer(
    event_t *event,
    uint16_t timeout_in_ms,
    bool repeating,
    bool immediate,
    closure_t *closure,
    uint32_t current_time
){
    event->type = TIMER_EVENT;
    event->closure = *closure;
    event->repeating = repeating;
    event->timer.due_time = immediate ?
        current_time :
        current_time + timeout_in_ms;
    event->timer.timeout = timeout_in_ms;
}
