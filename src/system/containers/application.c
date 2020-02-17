#include "application.h"

void app_init(application_t *app){
    syspools_init(&app->pools);
    sysqueues_init(&app->queues);
    sch_init(
        &app->scheduler,
        &app->pools,
        &app->queues
    );
    evloop_init(
        &app->event_loop,
        &app->pools,
        &app->queues
    );
    signal_relay_init(
        &app->relay,
        &app->pools,
        &app->queues,
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
        app->run_scheduler = false;
        sch_manage_timers(&app->scheduler);
    }
    evloop_run(&app->event_loop);
}

void app_run_later(
    application_t *app,
    uint16_t timeout_in_ms,
    closure_t closure
){
    sch_run_later(&app->scheduler, timeout_in_ms, closure);
}

void app_run_at_intervals(
  application_t *app,
  uint16_t interval_in_ms,
  bool immediate,
  closure_t closure
){
    sch_run_at_intervals(&app->scheduler, interval_in_ms, immediate, closure);
}

void app_enqueue_closure(application_t *app, closure_t *closure){
    evloop_enqueue_closure(&app->event_loop, closure);
}
