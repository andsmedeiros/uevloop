#include "event.h"

void event_destroy(event_t *event){
    switch (event->type){
        case CLOSURE_EVENT:
        case TIMER_EVENT:
            closure_destroy(&event->closure);
            break;
    }
}

void event_config_closure(event_t *event, closure_t *closure){
    event->type = CLOSURE_EVENT;
    event->closure = *closure;
}

void event_config_timer(
    event_t *event,
    uint16_t timeout_in_ms,
    bool repeating,
    closure_t *closure,
    uint32_t current_time
){
    event->type = TIMER_EVENT;
    event->timer.due_time = current_time + timeout_in_ms;
    event->timer.timeout = timeout_in_ms;
    event->timer.repeating = repeating;
    event->closure = *closure;
}
