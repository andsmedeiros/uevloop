/** \file scheduler.h
  * \brief Module responsible for keeping track of time and tasks to be run in
  * the future
  */

#ifndef UEL_SCHEDULER_H
#define	UEL_SCHEDULER_H

/// \cond
#include <stdint.h>
/// \endcond

#include "containers/system-pools.h"
#include "containers/system-queues.h"
#include "../utils/linked-list.h"
#include "../utils/closure.h"

/** \brief The scheduler object.
  *
  * This object keeps track of time run since the application was launched. It
  * also keeps a queue of events due to be processed sometime in the future.
  *
  * It feeds and is fed by the system queues. Timers due to be processed are put
  * in the outbound event queue. Timers awaiting scheduling are put in the
  * inbound schedule queue.
  */
typedef struct uel_scheduler uel_scheduer_t;
struct uel_scheduler{
    /** \brief Scheduled timers linked list
      *
      * This linked list holds events/timers scheduled to be run in the future.
      * Timers are inserted sorted by its due time, so it is always in
      * execution order.
      */
    uel_llist_t timer_list;

    /** \brief Paused timers linked list
      *
      * Holds events that had been scheduled but has been paused by the
      * programmer.
      * This is scanned for resumed timers every time `uel_sch_manage_timers`
      * is called.
      */
    uel_llist_t pause_list;

    uel_syspools_t *pools; //!< Reference to the system's pools
    uel_sysqueues_t *queues; //!< Reference to the system's queues

    /** \brief Internal timer. Must be updated via `uel_sch_update_timer()` */
    volatile uint32_t timer;
};

/** \brief Initialises a scheduler object
  *
  * \param scheduler The uel_scheduer_t instance to be initialised
  * \param pools The system's internal pools
  * \param queues The system's internal queues
  */
void uel_sch_init(
    uel_scheduer_t *scheduler,
    uel_syspools_t *pools,
    uel_sysqueues_t *queues
);

/** \brief Enqueues a closure for later execution.
  *
  * \param scheduler The uel_scheduer_t into which the event will be registered
  * \param timeout_in_ms The delay in milliseconds until the closure is run
  * \param closure The closure to be invoked when the due time is reached
  * \returns The scheduled event
  */
uel_event_t *uel_sch_run_later(
    uel_scheduer_t *scheduler,
    uint16_t  timeout_in_ms,
    uel_closure_t closure
);


/** \brief Enqueues a closure for execution at intervals.
  *
  * \param scheduler The uel_scheduer_t into which the event will be registered
  * \param interval_in_ms The delay in milliseconds two executions of the closure
  * \param immediate If this flag is set, the the event will be created with a
  * due time to the current time.
  * \param closure The closure to be invoked when the due time is reached
  * \returns The scheduled event
  */
uel_event_t *uel_sch_run_at_intervals(
    uel_scheduer_t *scheduler,
    uint16_t interval_in_ms,
    bool immediate,
    uel_closure_t closure
);

/** \brief Enqueue timers that are due to be processed in the event queue
  *
  * Checks, based on the current time counter, what timers should be enqueued for
  * execution. Afterwards, enqueue them.
  *
  * \param scheduler The uel_scheduer_t to manage
  */
void uel_sch_manage_timers(uel_scheduer_t *scheduler);

/** \brief Updates the internal time counter
  *
  * \param scheduler The scheduler whose time coounter should be updated
  * \param timer The new counter value to be acknowledged.
  */
void uel_sch_update_timer(uel_scheduer_t *scheduler, uint32_t timer);

#endif	/* UEL_SCHEDULER_H */
