#include "system-queues.h"
#include "../../portability/critical-section.h"

void sysqueues_init(sysqueues_t *queues){
    cqueue_init(
        &queues->event_queue,
        queues->event_queue_buffer,
        SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N
    );
    cqueue_init(
        &queues->schedule_queue,
        queues->schedule_queue_buffer,
        SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N
    );
}

void sysqueues_enqueue_event(sysqueues_t *queues, event_t *event){
    UEVLOOP_CRITICAL_ENTER;
    cqueue_push(&queues->event_queue, (void *)event);
    UEVLOOP_CRITICAL_EXIT;
}

event_t *sysqueues_get_enqueued_event(sysqueues_t *queues){
    event_t *event;
    UEVLOOP_CRITICAL_ENTER;
    event = (event_t *)cqueue_pop(&queues->event_queue);
    UEVLOOP_CRITICAL_EXIT;
    return event;
}

uintptr_t sysqueues_count_enqueued_events(sysqueues_t *queues){
    uintptr_t count;
    UEVLOOP_CRITICAL_ENTER;
    count = cqueue_count(&queues->event_queue);
    UEVLOOP_CRITICAL_EXIT;
    return count;
}

void sysqueues_schedule_event(sysqueues_t *queues, event_t *event){
    UEVLOOP_CRITICAL_ENTER;
    cqueue_push(&queues->schedule_queue, (void *)event);
    UEVLOOP_CRITICAL_EXIT;
}

event_t *sysqueues_get_scheduled_event(sysqueues_t *queues){
    event_t *event;
    UEVLOOP_CRITICAL_ENTER;
    event = (event_t *)cqueue_pop(&queues->schedule_queue);
    UEVLOOP_CRITICAL_EXIT;
    return event;
}

uintptr_t sysqueues_count_scheduled_events(sysqueues_t *queues){
    uintptr_t count;
    UEVLOOP_CRITICAL_ENTER;
    count = cqueue_count(&queues->schedule_queue);
    UEVLOOP_CRITICAL_EXIT;
    return count;
}
