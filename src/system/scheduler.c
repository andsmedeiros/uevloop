#include "uevloop/system/scheduler.h"

/// \cond
#include <stdlib.h>
/// \endcond

#include "uevloop/system/event.h"

static void *is_past_due_time(uel_closure_t *closure){
    uint32_t current_time = *(uint32_t *)closure->context;
    uel_llist_node_t *node = (uel_llist_node_t *)closure->params;
    uel_event_t *event = (uel_event_t *)node->value;
    bool fit_for_removal = event->detail.timer.due_time <= current_time;
    return (void *)fit_for_removal;
}

static void *place_in_order(uel_closure_t *closure){
    uint32_t due_time = *(uint32_t *)closure->context;
    uel_llist_node_t **nodes = (uel_llist_node_t **)closure->params;

    bool fits = false;
    if(nodes[1] == NULL){
        fits = true;
    }else if(nodes[0] == NULL){
        uel_event_t *next = (uel_event_t *)nodes[1]->value;
        fits = next->detail.timer.due_time > due_time;
    }else{
        uel_event_t *prev = (uel_event_t *)nodes[0]->value;
        uel_event_t *next = (uel_event_t *)nodes[1]->value;

        fits = prev->detail.timer.due_time <= due_time &&
            next->detail.timer.due_time > due_time;
    }

    return (void *)(uintptr_t)fits;
}

static void enqueue_timer(uel_scheduer_t *scheduler, uel_event_t *timer){
    uel_llist_node_t *node = uel_syspools_acquire_llist_node(scheduler->pools);
    node->value = (void *)timer;
    uel_closure_t in_order = uel_closure_create(
        place_in_order,
        (void *)&timer->detail.timer.due_time,
        NULL
    );
    uel_llist_insert_at(&scheduler->timer_list, node, &in_order);
}

static void reschedule_resumed_timers(uel_scheduer_t *scheduler){
    uel_llist_node_t *current = scheduler->pause_list.tail;
    while(current != NULL){
        uel_event_t *timer = (uel_event_t *)current->value;
        if (timer->detail.timer.status != UEL_TIMER_PAUSED) {
            uel_llist_remove(&scheduler->pause_list, current);
            timer->detail.timer.due_time =
                scheduler->timer + timer->detail.timer.timeout;
            uel_closure_t in_order = uel_closure_create(
                place_in_order,
                (void *)&timer->detail.timer.due_time,
                NULL
            );
            uel_llist_insert_at(&scheduler->timer_list, current, &in_order);
        }
        current = current->next;
    }
}

static void enqueue_expired_timers(uel_scheduer_t *scheduler){
    uel_closure_t closure =
        uel_closure_create(&is_past_due_time, (void *)&scheduler->timer, NULL);
    uel_llist_t expired_timers = uel_llist_remove_while(&scheduler->timer_list, &closure);
    uel_llist_node_t *current = expired_timers.tail;
    while(current != NULL){
        uel_event_t *timer = (uel_event_t *)current->value;
        if (timer->detail.timer.status == UEL_TIMER_PAUSED) {
            uel_llist_push_head(&scheduler->pause_list, current);
        }else{
            uel_syspools_release_llist_node(scheduler->pools, current);
            uel_sysqueues_enqueue_event(scheduler->queues, timer);
        }
        current = current->next;
    }
}

void uel_sch_init(
    uel_scheduer_t *scheduler,
    uel_syspools_t *pools,
    uel_sysqueues_t *queues
){
    uel_llist_init(&scheduler->timer_list);
    uel_llist_init(&scheduler->pause_list);
    scheduler->pools = pools;
    scheduler->queues = queues;
    scheduler->timer = 0;
}

uel_event_t *uel_sch_run_later(
    uel_scheduer_t *scheduler,
    uint16_t  timeout_in_ms,
    uel_closure_t closure
){
    uel_event_t *event = uel_syspools_acquire_event(scheduler->pools);
    uel_event_config_timer(event, timeout_in_ms, false, false, &closure, scheduler->timer);
    uel_sysqueues_schedule_event(scheduler->queues, event);
    return event;
}

uel_event_t *uel_sch_run_at_intervals(
    uel_scheduer_t *scheduler,
    uint16_t interval_in_ms,
    bool immediate,
    uel_closure_t closure
){
    uel_event_t *event = uel_syspools_acquire_event(scheduler->pools);
    uel_event_config_timer(event, interval_in_ms, true, immediate, &closure, scheduler->timer);
    if(immediate){
        uel_sysqueues_enqueue_event(scheduler->queues, event);
    }else{
        uel_sysqueues_schedule_event(scheduler->queues, event);
    }
    return event;
}

void uel_sch_manage_timers(uel_scheduer_t *scheduler){
    uel_event_t *event;
    while((event = uel_sysqueues_get_scheduled_event(scheduler->queues)) != NULL){
        enqueue_timer(scheduler, event);
    }

    reschedule_resumed_timers(scheduler);
    enqueue_expired_timers(scheduler);
}

void uel_sch_update_timer(uel_scheduer_t *scheduler, uint32_t timer){
    scheduler->timer = timer;
}
