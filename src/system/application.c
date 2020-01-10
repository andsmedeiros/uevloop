#include "application.h"

#include <stdlib.h>

void app_init(application_t *app){
    pools_init(&app->pools);
    cqueue_init(
        &app->event_queue,
        app->event_queue_buffer,
        APP_EVENT_QUEUE_SIZE_LOG2N
    );
    cqueue_init(
        &app->reschedule_queue,
        app->reschedule_queue_buffer,
        APP_RESCHEDULE_QUEUE_SIZE_LOG2N
    );
    sch_init(
        &app->scheduler,
        &app->pools.llist_node_pool,
        &app->pools.event_pool,
        &app->event_queue,
        &app->reschedule_queue
    );
    evloop_init(
        &app->event_loop,
        &app->pools.event_pool,
        &app->event_queue,
        &app->reschedule_queue
    );
    signal_relay_init(
        &app->relay,
        &app->event_loop,
        &app->pools.llist_node_pool,
        &app->pools.event_pool,
        app->relay_buffer,
        APP_EVENT_COUNT
    );
    app->run_scheduler = true;
}

void app_update_timer(application_t *app, uint32_t timer){
    sch_update_timer(&app->scheduler, timer);
    app->run_scheduler = true;
}

void app_tick(application_t *app){
    if(app->run_scheduler){
        sch_manage_timers(&app->scheduler);
        app->run_scheduler = false;
    }
    evloop_run(&app->event_loop);
}
