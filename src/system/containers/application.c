#include "application.h"

void uel_app_init(uel_application_t *app){
    uel_syspools_init(&app->pools);
    uel_sysqueues_init(&app->queues);
    uel_sch_init(
        &app->scheduler,
        &app->pools,
        &app->queues
    );
    uel_evloop_init(
        &app->event_loop,
        &app->pools,
        &app->queues
    );
   uel_signal_relay_init(
        &app->relay,
        &app->pools,
        &app->queues,
        app->relay_buffer,
        UEL_APP_EVENT_COUNT
    );
    app->run_scheduler = true;
}

void uel_app_update_timer(uel_application_t *app, uint32_t timer){
    uel_sch_update_timer(&app->scheduler, timer);
    app->run_scheduler = true;
}

void uel_app_tick(uel_application_t *app){
    if(app->run_scheduler){
        app->run_scheduler = false;
        uel_sch_manage_timers(&app->scheduler);
    }
    uel_evloop_run(&app->event_loop);
}

void uel_app_run_later(
    uel_application_t *app,
    uint16_t timeout_in_ms,
    uel_closure_t closure
){
    uel_sch_run_later(&app->scheduler, timeout_in_ms, closure);
}

void uel_app_run_at_intervals(
  uel_application_t *app,
  uint16_t interval_in_ms,
  bool immediate,
  uel_closure_t closure
){
    uel_sch_run_at_intervals(&app->scheduler, interval_in_ms, immediate, closure);
}

void uel_app_enqueue_closure(uel_application_t *app, uel_closure_t *closure){
    uel_evloop_enqueue_closure(&app->event_loop, closure);
}
