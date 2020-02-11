/** \file scheduler.h
  * \brief Module responsible for keeping track of time and tasks to be run in
  * the future
  */

#ifndef SCHEDULER_H
#define	SCHEDULER_H

#include <stdint.h>
#include "event.h"
#include "syspools.h"
#include "../utils/linked-list.h"
#include "../utils/closure.h"
#include "../utils/circular-queue.h"

/** \brief The scheduler object.
  *
  * This object keeps track of time run since the application was launched. It
  * also keeps a queue of events due to be processed sometime in the future.
  */
typedef struct scheduler scheduler_t;
struct scheduler{
    /** \brief Scheduled timers linked list
      *
      * This linked list holds events/timers scheduled to be run in the future.
      * Timers are inserted sorted by its due time, so it is always in
      * execution order.
      *
      * Expired timers may be returned here if they are repeating.
      */
    llist_t timer_list;

    syspools_t *pools; //!< Reference to the system's pools

    /** \brief The system's event queue
      *
      * When an event's due time arrive, they will put in this queue for further
      * collection and processing by the event loop.
      */
    cqueue_t *event_queue;

    /** \brief The system's reschedule queue
      *
      * Events put in this queue will be reschedule according to their due time
      * by the next time the scheduler manages its timers.
      */
    cqueue_t *reschedule_queue;

    /** \brief Internal timer. Must be updated via `evloop_update_timer()` */
    volatile uint32_t timer;
};

/** \brief Initialises a scheduler object
  *
  * \param scheduler The scheduler_t instance to be initialised
  * \param pools The system's internal pools
  * \param event_queue The queue into which events ready to be run are put
  * \param reschedule_queue The queue that holds events ready for rescheduling
  */
void sch_init(
    scheduler_t *scheduler,
    syspools_t *pools,
    cqueue_t *event_queue,
    cqueue_t *reschedule_queue
);

/** \brief Enqueues a closure for later execution.
  *
  * \param scheduler The scheduler_t into which the event will be registered
  * \param timeout_in_ms The delay in milliseconds until the closure is run
  * \param closure The closure to be invoked when the due time is reached
  */
void sch_run_later(
    scheduler_t *scheduler,
    uint16_t  timeout_in_ms,
    closure_t closure
);


/** \brief Enqueues a closure for execution at intervals.
  *
  * \param scheduler The scheduler_t into which the event will be registered
  * \param interval_in_ms The delay in milliseconds two executions of the closure
  * \param immediate If this flag is set, the the event will be created with a
  * due time to the current time.
  * \param closure The closure to be invoked when the due time is reached
  */
void sch_run_at_intervals(
    scheduler_t *scheduler,
    uint16_t interval_in_ms,
    bool immediate,
    closure_t closure
);

/** \brief Enqueue timers that are due to be processed in the event queue
  *
  * Checks, based on the current time counter, what timers should be enqueued for
  * execution. Afterwards, enqueue them.
  *
  * \param scheduler The scheduler_t to manage
  */
void sch_manage_timers(scheduler_t *scheduler);

/** \brief Reschedules an already initialised timer
  *
  * \param scheduler The scheduler into which to reschedule the timer
  * \param timer The timer to be rescheduled
  */
void sch_reschedule(scheduler_t *scheduler, event_t *timer);

/** \brief Updates the internal time counter
  *
  * \param scheduler The scheduler whose time coounter should be updated
  * \param timer The new counter value to be acknowledged.
  */
void sch_update_timer(scheduler_t *scheduler, uint32_t timer);

#endif	/* SCHEDULER_H */
