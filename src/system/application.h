/**  \file application.h
   * \brief The application module is a convenient top-level container for the
   * system internals.
   */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "pools.h"
#include "event-loop.h"
#include "scheduler.h"
#include "signal.h"
#include "../utils/circular-queue.h"

//! Events emitted by the application relay. Unused ATM.
enum app_event{
    APP_READY = 0,
    APP_CRASHED,
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
    pools_t pools; //!< Holds the system pools: events and llist nodes

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

/** \brief Updates the internal timer of an application, located at the scheduler
    \param app The application_t instance
    \param timer The current application timer, in milliseconds
  */
void app_update_timer(application_t *app, uint32_t timer);

/** \brief Ticks the application.
  *
  * Yields control to the application runtime. This will:
  * 1. Check if the scheduler ought to be run (i.e.:  the counter has been
  * updated or there are events awaiting rescheduling) and do so if necessary
  * 2. Perform a runloop
  *
  * \param app The application_t instance
  */
void app_tick(application_t *app);

#endif /* end of include guard: APPLICATION_H */
