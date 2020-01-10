#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "pools.h"
#include "event-loop.h"
#include "scheduler.h"
#include "signal.h"

enum app_event{
    APP_READY = 0,
    APP_CRASHED,
    APP_EVENT_COUNT
};
typedef enum app_event app_event_t;

#define APP_EVENT_QUEUE_SIZE_LOG2N (5)
#define APP_EVENT_QUEUE_SIZE (1<<APP_EVENT_QUEUE_SIZE_LOG2N)

#define APP_RESCHEDULE_QUEUE_SIZE_LOG2N (4)
#define APP_RESCHEDULE_QUEUE_SIZE (1<<APP_RESCHEDULE_QUEUE_SIZE_LOG2N)

typedef struct application application_t;
struct application{
    pools_t pools;

    evloop_t event_loop;
    scheduler_t scheduler;

    signal_relay_t relay;
    llist_t relay_buffer[APP_EVENT_COUNT];

    void *event_queue_buffer[APP_EVENT_QUEUE_SIZE];
    cqueue_t event_queue;

    void *reschedule_queue_buffer[APP_RESCHEDULE_QUEUE_SIZE];
    cqueue_t reschedule_queue;

    bool run_scheduler;
};

void app_init(application_t *app);
void app_update_timer(application_t *app, uint32_t timer);
void app_tick(application_t *app);

#endif /* end of include guard: APPLICATION_H */
