#include "scheduler.h"
#include "event.h"

static void update_timer(closure_t *closure){
    scheduler_t *scheduler = (scheduler_t *)closure->context;
    scheduler->timer++;
}

static void should_insert(closure_t *closure){
    event_t *timer = (event_t *)closure->context;
    llist_node_t **nodes = (llist_node_t **)closure->params;
    bool fit_for_insertion = true;
    
    if(nodes[1] != NULL){
        event_t *prev = (event_t *)nodes[0]->value;
        event_t *next = (event_t *)nodes[1]->value;
        fit_for_insertion = 
            prev->timer.due_time <= timer->timer.due_time &&
            next->timer.due_time > timer->timer.due_time;
    }
    closure_return(closure, (void *)fit_for_insertion);
}

static void should_remove(closure_t *closure){
    
}

static void enqueue_timer(scheduler_t *scheduler, event_t *timer){
    llist_node_t *node = (llist_node_t *)objpool_acquire(scheduler->llist_node_pool);
    node->value = (void *)timer;
    closure_t closure = closure_create(&should_insert, timer, NULL);
    llist_insert_at(&scheduler->timer_list, node, &closure);
}

static void manage(scheduler_t *scheduler){
    
}

void sch_init(
    scheduler_t *scheduler, 
    pools_t *pools,
    void (*on_timer_tick)(closure_t)
){
    cqueue_init(
        &scheduler->event_queue, 
        scheduler->event_queue_buffer, 
        SCHEDULER_EVENT_QUEUE_SIZE_LOG2N
    );
    
    llist_init(&scheduler->timer_list);
    
    scheduler->event_pool = pools_get(pools, EVENT_POOL);
    scheduler->llist_node_pool = pools_get(pools, LLIST_NODE_POOL);
    scheduler->timer = 0;
    
    on_timer_tick(closure_create(&update_timer, scheduler, NULL));
}

void sch_tick(scheduler_t *scheduler){
    manage(scheduler);
    while(!cqueue_is_empty(&scheduler->event_queue)){
        event_t *event = cqueue_pop(&scheduler->event_queue);
        switch(event->type){
            case CLOSURE_EVENT: 
                closure_invoke(&event->closure, scheduler);
                break;
        }
        event_destroy(event);
        objpool_release(scheduler->event_pool, (void *)event);
    }
}

void sch_enqueue(scheduler_t *scheduler, closure_t closure){
    event_t *event = (event_t *)objpool_acquire(scheduler->event_pool);
    event_config_closure(event, &closure);
    cqueue_push(&scheduler->event_queue, (void *)event);
}

void sch_run_later(
    scheduler_t *scheduler, 
    uint16_t  timeout_in_ms, 
    closure_t closure
){
    event_t *event = (event_t *)objpool_acquire(scheduler->event_pool);
    event_config_timer(event, timeout_in_ms, false, &closure, scheduler->timer);
}

void sch_run_at_intervals(
    scheduler_t *scheduler, 
    uint16_t interval_in_ms, 
    bool immediate, 
    closure_t closure
){
    if(immediate){
        sch_enqueue(scheduler, closure);
    }
    event_t *event = (event_t *)objpool_acquire(scheduler->event_pool);
    event_config_timer(event, interval_in_ms, true, &closure, scheduler->timer);
}