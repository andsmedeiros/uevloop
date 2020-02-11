#include "application.h"

#include <stdlib.h>
#include "../portability/critical-section.h"

void app_init(application_t *app){
    syspools_init(&app->pools);
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
        &app->pools,
        &app->event_queue,
        &app->reschedule_queue
    );
    evloop_init(
        &app->event_loop,
        &app->pools,
        &app->event_queue,
        &app->reschedule_queue
    );
    signal_relay_init(
        &app->relay,
        &app->event_loop,
        &app->pools,
        app->relay_buffer,
        APP_EVENT_COUNT
    );
    app->run_scheduler = true;
}

void app_update_timer(application_t *app, uint32_t timer){
    UEVLOOP_CRITICAL_ENTER;
    sch_update_timer(&app->scheduler, timer);
    UEVLOOP_CRITICAL_EXIT;
    app->run_scheduler = true;
}

void app_tick(application_t *app){
    if(app->run_scheduler){
        UEVLOOP_CRITICAL_ENTER;
        sch_manage_timers(&app->scheduler);
        UEVLOOP_CRITICAL_EXIT;
        app->run_scheduler = false;
    }
    UEVLOOP_CRITICAL_ENTER;
    evloop_run(&app->event_loop);
    UEVLOOP_CRITICAL_EXIT;
}

void app_run_later(
    application_t *app,
    uint16_t timeout_in_ms,
    closure_t closure
){
    UEVLOOP_CRITICAL_ENTER;
    sch_run_later(&app->scheduler, timeout_in_ms, closure);
    UEVLOOP_CRITICAL_EXIT;
}

void app_run_at_intervals(
  application_t *app,
  uint16_t interval_in_ms,
  bool immediate,
  closure_t closure
){
    UEVLOOP_CRITICAL_ENTER;
    sch_run_at_intervals(&app->scheduler, interval_in_ms, immediate, closure);
    UEVLOOP_CRITICAL_EXIT;
}

void app_enqueue_closure(application_t *app, closure_t *closure){
    UEVLOOP_CRITICAL_ENTER;
    evloop_enqueue_closure(&app->event_loop, closure);
    UEVLOOP_CRITICAL_EXIT;
}
