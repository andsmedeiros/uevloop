#include "event-loop.h"

void evloop_init(
    evloop_t *event_loop,
    objpool_t *event_pool,
    cqueue_t *event_queue,
    cqueue_t *reschedule_queue
){
    event_loop->event_pool = event_pool;
    event_loop->event_queue = event_queue;
    event_loop->reschedule_queue = reschedule_queue;
}

void evloop_run(evloop_t *event_loop){
    while(!cqueue_is_empty(event_loop->event_queue)){
        event_t *event = cqueue_pop(event_loop->event_queue);
        switch(event->type){
            case CLOSURE_EVENT:
                closure_invoke(&event->closure, event_loop);
                break;
            case TIMER_EVENT:
                closure_invoke(&event->closure, event_loop);
                if (event->repeating) {
                    cqueue_push(event_loop->reschedule_queue, (void *)event);
                    continue;
                }
                break;
            case SIGNAL_EVENT:
                closure_invoke(&event->closure, event->closure.params);
                continue;
            default: break;
        }
        event_destroy(event);
        objpool_release(event_loop->event_pool, (void *)event);
    }
}

void evloop_enqueue_event(evloop_t *event_loop, event_t *event){
    cqueue_push(event_loop->event_queue, (void *)event);
}

void evloop_enqueue_closure(evloop_t *event_loop, closure_t *closure){
    event_t *event = (event_t *)objpool_acquire(event_loop->event_pool);
    event_config_closure(event, closure);
    evloop_enqueue_event(event_loop, event);
}
