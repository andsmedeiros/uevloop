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
    app->registry = NULL;
    app->registry_size = 0;
}

void uel_app_boot(uel_application_t *app, uel_module_t **modules, size_t module_count){
    for (size_t i = 0; i < module_count; i++) {
        uel_module_config(modules[i]);
    }
    for (size_t i = 0; i < module_count; i++) {
        uel_module_launch(modules[i]);
    }
    app->registry_size = module_count;
    app->registry = modules;
}

uel_module_t *uel_app_require(uel_application_t *app, size_t id){
    return app->registry[id];
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

uel_event_t *uel_app_run_later(
    uel_application_t *app,
    uint16_t timeout_in_ms,
    uel_closure_t closure
){
    app->run_scheduler = true;
    return uel_sch_run_later(&app->scheduler, timeout_in_ms, closure);
}

uel_event_t *uel_app_run_at_intervals(
  uel_application_t *app,
  uint16_t interval_in_ms,
  bool immediate,
  uel_closure_t closure
){
    app->run_scheduler = true;
    return uel_sch_run_at_intervals(&app->scheduler, interval_in_ms, immediate, closure);
}

void uel_app_enqueue_closure(uel_application_t *app, uel_closure_t *closure){
    uel_evloop_enqueue_closure(&app->event_loop, closure);
}

uel_event_t *uel_app_observe(
    uel_application_t *app,
    volatile uintptr_t *condition_var,
    uel_closure_t *closure
){
    return uel_evloop_observe(&app->event_loop, condition_var, closure);
}
