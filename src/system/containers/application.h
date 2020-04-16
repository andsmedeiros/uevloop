/**  \file application.h
   * \brief The application module is a convenient top-level container for the
   * system internals.
   */

#ifndef UEL_APPLICATION_H
#define UEL_APPLICATION_H
/// \cond
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
/// \endcond

#include "system-pools.h"
#include "system-queues.h"
#include "system/event-loop.h"
#include "system/scheduler.h"
#include "system/signal.h"
#include "utils/module.h"

//! Events emitted by the application relay. Unused ATM.
enum uel_app_event{
    UEL_APP_READY = 0,
    UEL_APP_CRASHED,
    UEL_APP_IDLE,
    UEL_APP_EVENT_COUNT
};
//! Alias to the uel_app_event enum
typedef enum uel_app_event uel_app_event_t;

/** \brief Top-level container for µEvLoop'd application
  *
  * The application module is not necessary, but it does facilitate creating
  * and managing the state of a µEvLoop application.
  * It contains all the system insternals and manages the scheduler and event
  * loops queues automatically.
  */
typedef struct uel_application uel_application_t;
struct uel_application{
    uel_module_t **registry; //!< The modules managed by this application
    size_t registry_size; //!< The number of modules managed by this application
    uel_syspools_t pools; //!< Holds the system pools: events and llist nodes
    uel_sysqueues_t queues; //!< Holds the system event queues
    uel_evloop_t event_loop; //!< The application's event loop
    uel_scheduer_t scheduler;  //!< The applications's scheduler;
    uel_signal_relay_t relay;   //!< Unused
    uel_llist_t relay_buffer[UEL_APP_EVENT_COUNT]; //!< Unused
    bool run_scheduler; //!< Marks when it's time to wake the scheduler
};

/** \brief Initialises an uel_application_t instance
  * \param app The uel_application_t instance
  */
void uel_app_init(uel_application_t *app);

/** \brief Boots an application, initiliasing each module registered
  *
  * \param app The application to be booted
  * \param modules The modules to be loaded by the application
  * \param module_count The number of modules being loaded
  */
void uel_app_boot(uel_application_t *app, uel_module_t **modules, size_t module_count);

/** \brief Fetches a module from the app's registry
  *
  * \param app The application from where to fetch the module
  * \param id The module ID  to be fetched
  *
  * \returns The module at `id`th position in the registry
  */
uel_module_t *uel_app_require(uel_application_t *app, size_t id);

/** \brief Ticks the application.
  *
  * Yields control to the application runtime. This will:
  * 1. Check if the scheduler ought to be run (i.e.:  the counter has been
  * updated or there are events awaiting rescheduling) and do so if necessary
  * 2. Perform a runloop
  *
  * \param app The uel_application_t instance
  */
void uel_app_tick(uel_application_t *app);

/** \brief Updates the internal timer of an application, located at the scheduler
  *
  * \param app The uel_application_t instance
  * \param timer The current application timer, in milliseconds
  */
void uel_app_update_timer(uel_application_t *app, uint32_t timer);

/** \brief Enqueues a closure for later execution.
  *
  * Proxies the call to uel_sch_run_later() with uel_application_t::scheduler as
  * parameter.
  *
  * \param app The uel_application_t instance
  * \param timeout_in_ms The delay in milliseconds until the closure is run
  * \param closure The closure to be invoked when the due time is reached
  */
  void uel_app_run_later(
      uel_application_t *app,
      uint16_t timeout_in_ms,
      uel_closure_t closure
  );

/** \brief Enqueues a closure for execution at intervals.
  *
  * Proxies the call to uel_sch_run_at_intervals() with uel_application_t::scheduler as
  * parameter.
  *
  * \param app The uel_application_t instance
  * \param interval_in_ms The delay in milliseconds two executions of the closure
  * \param immediate If this flag is set, the the event will be created with a
  * due time to the current time.
  * \param closure The closure to be invoked when the due time is reached
  */
void uel_app_run_at_intervals(
    uel_application_t *app,
    uint16_t interval_in_ms,
    bool immediate,
    uel_closure_t closure
);

/** \brief Enqueues a closure to be invoked.
  *
  * Proxies the call to uel_evloop_enqueue_closure() with uel_application_t::event_loop
  * as parameter.
  *
  * \param app The uel_application_t instance
  * \param closure The closure to be enqueued
  */
void uel_app_enqueue_closure(uel_application_t *app, uel_closure_t *closure);

#endif /* end of include guard: UEL_APPLICATION_H */
