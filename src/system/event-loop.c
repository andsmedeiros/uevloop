#include "event-loop.h"

/// \cond
#include <stdlib.h>
/// \endcond

void evloop_init(
    evloop_t *event_loop,
    syspools_t *pools,
    sysqueues_t *queues
){
    event_loop->pools = pools;
    event_loop->queues = queues;
}

void evloop_run(evloop_t *event_loop){
    event_t *event;
    while((event = sysqueues_get_enqueued_event(event_loop->queues)) != NULL){
        switch(event->type){
            case CLOSURE_EVENT:
                closure_invoke(&event->closure, event_loop);
                break;
            case TIMER_EVENT:
                closure_invoke(&event->closure, event_loop);
                if (event->repeating) {
                    event->detail.timer.due_time += event->detail.timer.timeout;
                    sysqueues_schedule_event(event_loop->queues, event);
                    continue;
                }
                break;
            case SIGNAL_EVENT:
                closure_invoke(&event->closure, event->closure.params);
                if(event->repeating){
                    continue;
                }
            default: break;
        }
        event_destroy(event);
        syspools_release_event(event_loop->pools, event);
    }
}

void evloop_enqueue_closure(evloop_t *event_loop, closure_t *closure){
    event_t *event = syspools_acquire_event(event_loop->pools);
    event_config_closure(event, closure);
    sysqueues_enqueue_event(event_loop->queues, event);
}
