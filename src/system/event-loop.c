#include "event-loop.h"

/// \cond
#include <stdlib.h>
#include <stdbool.h>
/// \endcond

#include "../config.h"
#include "../portability/critical-section.h"

static inline bool run_closure_event(evloop_t *event_loop, event_t *event){
    closure_invoke(&event->closure, event_loop);
    return event->repeating;
}

static inline bool run_timer_event(evloop_t *event_loop, event_t *event){
    closure_invoke(&event->closure, event_loop);
    if (event->repeating) {
        event->detail.timer.due_time += event->detail.timer.timeout;
        sysqueues_schedule_event(event_loop->queues, event);
        return true;
    }
    return false;
}

static inline void run_signal_event(evloop_t *event_loop, event_t *signal){
    closure_t closures[SIGNAL_MAX_LISTENERS];
    llist_node_t *removed_nodes[SIGNAL_MAX_LISTENERS];
    llist_t *listeners = signal->detail.signal.listeners;
    unsigned int i = 0, j =  0;

    UEVLOOP_CRITICAL_ENTER;
    for(llist_node_t *current = listeners->tail;
        current != NULL && i < SIGNAL_MAX_LISTENERS;
        current = current->next
    ){
        event_t *listener = (event_t *)current->value;
        if(!listener->repeating || listener->detail.listener.unlistened){
            llist_remove(listeners, current);
            removed_nodes[j++] = current;
        }
        if(!listener->detail.listener.unlistened){
            closures[i++] = listener->closure;
        }
    }
    UEVLOOP_CRITICAL_EXIT;

    for(unsigned int closure_count = i, i = 0; i < closure_count; i++){
        closure_t *closure = &closures[i];
        closure_invoke(closure, signal->closure.params);
    }
    for(unsigned int node_count = j, j = 0; j < node_count; j++){
        event_t *event = removed_nodes[j]->value;
        event_destroy(event);
        syspools_release_event(event_loop->pools, event);
        syspools_release_llist_node(event_loop->pools, removed_nodes[j]);
    }
}

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
                if(run_closure_event(event_loop, event)) continue;
                break;
            case TIMER_EVENT:
                if(run_timer_event(event_loop, event)) continue;
                break;
            case SIGNAL_EVENT:
                run_signal_event(event_loop, event);
                break;
            default: continue;
        }
        event_destroy(event);
        syspools_release_event(event_loop->pools, event);
    }
}

void evloop_enqueue_closure(evloop_t *event_loop, closure_t *closure){
    event_t *event = syspools_acquire_event(event_loop->pools);
    event_config_closure(event, closure, false);
    sysqueues_enqueue_event(event_loop->queues, event);
}
