#include "uevloop/system/containers/system-queues.h"
#include "uevloop/portability/critical-section.h"

void uel_sysqueues_init(uel_sysqueues_t *queues){
    uel_cqueue_init(
        &queues->event_queue,
        queues->event_queue_buffer,
        UEL_SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N
    );
    uel_cqueue_init(
        &queues->schedule_queue,
        queues->schedule_queue_buffer,
        UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N
    );
}

void uel_sysqueues_enqueue_event(uel_sysqueues_t *queues, uel_event_t *event){
    UEL_CRITICAL_ENTER;
    uel_cqueue_push(&queues->event_queue, (void *)event);
    UEL_CRITICAL_EXIT;
}

uel_event_t *uel_sysqueues_get_enqueued_event(uel_sysqueues_t *queues){
    uel_event_t *event;
    UEL_CRITICAL_ENTER;
    event = (uel_event_t *)uel_cqueue_pop(&queues->event_queue);
    UEL_CRITICAL_EXIT;
    return event;
}

uintptr_t uel_sysqueues_count_enqueued_events(uel_sysqueues_t *queues){
    uintptr_t count;
    UEL_CRITICAL_ENTER;
    count = uel_cqueue_count(&queues->event_queue);
    UEL_CRITICAL_EXIT;
    return count;
}

void uel_sysqueues_schedule_event(uel_sysqueues_t *queues, uel_event_t *event){
    UEL_CRITICAL_ENTER;
    uel_cqueue_push(&queues->schedule_queue, (void *)event);
    UEL_CRITICAL_EXIT;
}

uel_event_t *uel_sysqueues_get_scheduled_event(uel_sysqueues_t *queues){
    uel_event_t *event;
    UEL_CRITICAL_ENTER;
    event = (uel_event_t *)uel_cqueue_pop(&queues->schedule_queue);
    UEL_CRITICAL_EXIT;
    return event;
}

uintptr_t uel_sysqueues_count_scheduled_events(uel_sysqueues_t *queues){
    uintptr_t count;
    UEL_CRITICAL_ENTER;
    count = uel_cqueue_count(&queues->schedule_queue);
    UEL_CRITICAL_EXIT;
    return count;
}
