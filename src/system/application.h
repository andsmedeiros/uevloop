/**  \file application.h
   * \brief The application module is a convenient top-level container for the
   * system internals.
   * It also serves as a thread/interrupt safe proxy interface.
   */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "syspools.h"
#include "event-loop.h"
#include "scheduler.h"
#include "signal.h"
#include "../utils/circular-queue.h"

//! Events emitted by the application relay. Unused ATM.
enum app_event{
    APP_READY = 0,
    APP_CRASHED,
    APP_IDLE,
    APP_EVENT_COUNT
};
//! Alias to the app_event enum
typedef enum app_event app_event_t;

//! The size of the event queue in log2 form
#define APP_EVENT_QUEUE_SIZE_LOG2N (5)
//! Unrolls the `APP_EVENT_QUEUE_SIZE_LOG2N` value to its power-of-two form
#define APP_EVENT_QUEUE_SIZE (1<<APP_EVENT_QUEUE_SIZE_LOG2N)

//! The size of the reschedule queue in log2 form
#define APP_RESCHEDULE_QUEUE_SIZE_LOG2N (4)
//! Unrolls the `APP_RESCHEDULE_QUEUE_SIZE_LOG2N` value to its power-of-two form
#define APP_RESCHEDULE_QUEUE_SIZE (1<<APP_RESCHEDULE_QUEUE_SIZE_LOG2N)

/** \brief Top-level container for µEvLoop'd application
  *
  * The application module is not necessary, but it does facilitate creating
  * and managing the state of a µEvLoop application.
  * It contains all the system insternals and manages the scheduler and event
  * loops queues automatically.
  */
typedef struct application application_t;
struct application{
    syspools_t pools; //!< Holds the system pools: events and llist nodes

    evloop_t event_loop; //!< The application's event loop
    scheduler_t scheduler;  //!< The applications's scheduler;

    signal_relay_t relay;   //!< Unused
    llist_t relay_buffer[APP_EVENT_COUNT]; //!< Unused

    //! The event queue buffer
    void *event_queue_buffer[APP_EVENT_QUEUE_SIZE];
    cqueue_t event_queue;   /**< \brief The application's event queue.
                              *
                              * Holds events ready to be processed on the next
                              * runloop.
                              */

    //! The reschedule queue buffer
    void *reschedule_queue_buffer[APP_RESCHEDULE_QUEUE_SIZE];
    cqueue_t reschedule_queue; /**< \brief The application's reschedule queue.
                                 *
                                 * Hold events already processed by the runloop
                                 * but fit for rescheduling at the scheduler.
                                 */

    bool run_scheduler; //!< Marks when it's time to wake the scheduler
};

/** \brief Initialises an application_t instance
  * \param app The application_t instance
  */
void app_init(application_t *app);

/** \brief Ticks the application.
  *
  * Yields control to the application runtime. This will:
  * 1. Check if the scheduler ought to be run (i.e.:  the counter has been
  * updated or there are events awaiting rescheduling) and do so if necessary
  * 2. Perform a runloop
  *
  * \note This function is run in a critical section
  *
  * \param app The application_t instance
  */
void app_tick(application_t *app);

/** \brief Updates the internal timer of an application, located at the scheduler
  *
  * \param app The application_t instance
  * \param timer The current application timer, in milliseconds
  */
void app_update_timer(application_t *app, uint32_t timer);

/** \brief Enqueues a closure for later execution.
  *
  * Proxies the call to sch_run_later() with application_t::scheduler as
  * parameter.
  *
  * \note This function is run in a critical section
  *
  * \param app The application_t instance
  * \param timeout_in_ms The delay in milliseconds until the closure is run
  * \param closure The closure to be invoked when the due time is reached
  */
  void app_run_later(
      application_t *app,
      uint16_t timeout_in_ms,
      closure_t closure
  );

/** \brief Enqueues a closure for execution at intervals.
  *
  * Proxies the call to sch_run_at_intervals() with application_t::scheduler as
  * parameter.
  *
  * \note This function is run in a critical section
  *
  * \param app The application_t instance
  * \param interval_in_ms The delay in milliseconds two executions of the closure
  * \param immediate If this flag is set, the the event will be created with a
  * due time to the current time.
  * \param closure The closure to be invoked when the due time is reached
  */
void app_run_at_intervals(
    application_t *app,
    uint16_t interval_in_ms,
    bool immediate,
    closure_t closure
);

/** \brief Enqueues a closure to be invoked.
  *
  * Proxies the call to evloop_enqueue_closure() with application_t::event_loop
  * as parameter.
  *
  * \note This function is run in a critical section
  *
  * \param app The application_t instance
  * \param closure The closure to be enqueued
  */
void app_enqueue_closure(application_t *app, closure_t *closure);

#endif /* end of include guard: APPLICATION_H */
