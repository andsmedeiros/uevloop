#include "uevloop/system/event-loop.h"

/// \cond
#include <stdlib.h>
#include <stdbool.h>
/// \endcond

#include "uevloop/config.h"
#include "uevloop/utils/iterator.h"
#include "uevloop/portability/critical-section.h"

static inline bool run_closure_event(uel_evloop_t *event_loop, uel_event_t *event){
    uel_closure_invoke(&event->closure, event->value);
    return event->repeating;
}

static inline bool run_timer_event(uel_evloop_t *event_loop, uel_event_t *event){
    switch (event->detail.timer.status) {
        case UEL_TIMER_CANCELLED:
            return false;
        case UEL_TIMER_PAUSED:
            uel_sysqueues_schedule_event(event_loop->queues, event);
            return true;
        default: break;
    }
    uel_closure_invoke(&event->closure, event->value);
    if (event->repeating) {
        event->detail.timer.due_time += event->detail.timer.timeout;
        uel_sysqueues_schedule_event(event_loop->queues, event);
        return true;
    }
    return false;
}

static inline void run_signal_event(uel_evloop_t *event_loop, uel_event_t *signal){
    uel_closure_t closures[UEL_SIGNAL_MAX_LISTENERS];
    uel_llist_node_t *removed_nodes[UEL_SIGNAL_MAX_LISTENERS];
    uel_llist_t *listeners = signal->detail.signal.listeners;
    unsigned int i = 0, j =  0;

    UEL_CRITICAL_ENTER;
    for(uel_llist_node_t *current = listeners->tail;
        current != NULL && i < UEL_SIGNAL_MAX_LISTENERS;
        current = current->next
    ){
        uel_event_t *listener = (uel_event_t *)current->value;
        if(!listener->repeating || listener->detail.listener.unlistened){
            uel_llist_remove(listeners, current);
            removed_nodes[j++] = current;
        }
        if(!listener->detail.listener.unlistened){
            closures[i++] = listener->closure;
        }
    }
    UEL_CRITICAL_EXIT;

    for(unsigned int uel_closure_count = i, i = 0; i < uel_closure_count; i++){
        uel_closure_t *closure = &closures[i];
        uel_closure_invoke(closure, signal->value);
    }
    for(unsigned int node_count = j, j = 0; j < node_count; j++){
        uel_event_t *event = removed_nodes[j]->value;
        uel_syspools_release_event(event_loop->pools, event);
        uel_syspools_release_llist_node(event_loop->pools, removed_nodes[j]);
    }
}

static void *run_observer_event(void *context, void *params){
    uel_evloop_t *event_loop = (uel_evloop_t *)context;
    uel_llist_node_t *node = (uel_llist_node_t *)params;
    uel_event_t *event = (uel_event_t *)node->value;
    struct uel_event_observer *observer = &event->detail.observer;

    if(!observer->cancelled){
        uintptr_t value;
        // Ensures lock-free synchronisation
        do{
            value = *observer->condition_var;
        } while(value != *observer->condition_var);

        if(value != observer->last_value){
            uel_closure_invoke(&event->closure, (void *)value);
            observer->last_value = value;
        }
    }

    if (observer->cancelled || !event->repeating) {
        uel_llist_remove(&event_loop->observers, node);
        uel_syspools_release_event(event_loop->pools, event);
        uel_syspools_release_llist_node(event_loop->pools, node);
    }

    return (void *)true;
}

static void register_observer(uel_evloop_t *event_loop, uel_event_t *observer){
    uel_llist_node_t *node = uel_syspools_acquire_llist_node(event_loop->pools);
    node->value = (void *)observer;
    UEL_CRITICAL_ENTER;
    uel_llist_push_head(&event_loop->observers, node);
    UEL_CRITICAL_EXIT;
}

void uel_evloop_init(
    uel_evloop_t *event_loop,
    uel_syspools_t *pools,
    uel_sysqueues_t *queues
){
    event_loop->pools = pools;
    event_loop->queues = queues;
    uel_llist_init(&event_loop->observers);
}

void uel_evloop_run(uel_evloop_t *event_loop){
    uel_event_t *event;
    while((event = uel_sysqueues_get_enqueued_event(event_loop->queues)) != NULL){
        switch(event->type){
            case UEL_CLOSURE_EVENT:
                if(run_closure_event(event_loop, event)) continue;
                break;
            case UEL_TIMER_EVENT:
                if(run_timer_event(event_loop, event)) continue;
                break;
            case UEL_SIGNAL_EVENT:
                run_signal_event(event_loop, event);
                break;
            default: continue;
        }
        uel_syspools_release_event(event_loop->pools, event);
    }

    uel_closure_t observe =
        uel_closure_create(run_observer_event, (void *)event_loop);
    uel_iterator_llist_t observer_it =
        uel_iterator_llist_create(&event_loop->observers);
    uel_iterator_foreach(&observer_it, &observe);
}

void uel_evloop_enqueue_closure(
    uel_evloop_t *event_loop,
    uel_closure_t *closure,
    void *value
){
    uel_event_t *event = uel_syspools_acquire_event(event_loop->pools);
    uel_event_config_closure(event, closure, value, false);
    uel_sysqueues_enqueue_event(event_loop->queues, event);
}


uel_event_t *uel_evloop_observe(
  uel_evloop_t *event_loop,
  volatile uintptr_t *condition_var,
  uel_closure_t *closure
){
    uel_event_t *observer = uel_syspools_acquire_event(event_loop->pools);
    uel_event_config_observer(observer, closure, condition_var, true);
    register_observer(event_loop, observer);

    return observer;
}

uel_event_t *uel_evloop_observe_once(
  uel_evloop_t *event_loop,
  volatile uintptr_t *condition_var,
  uel_closure_t *closure
){
    uel_event_t *observer = uel_syspools_acquire_event(event_loop->pools);
    uel_event_config_observer(observer, closure, condition_var, false);
    register_observer(event_loop, observer);

    return observer;
}
