/** \file system-queues.h
  * \brief A container for the system's internal queues
  */

#ifndef UEL_SYSTEM_QUEUES_H
#define UEL_SYSTEM_QUEUES_H

/// \cond
#include <stdint.h>
/// \endcond

#include "uevloop/system/event.h"
#include "uevloop/config.h"
#include "uevloop/utils/circular-queue.h"

/** \brief A container for the system's internal queues
  *
  * This module conveniently declares and contains the object queues necessary for
  * internal operation of the application.
  *
  * It also encapsulate manipulation of shared memory in critical sections. All
  * of its functions are safe, except for `uel_sysqueues_init`.
  */
typedef struct sysqueues uel_sysqueues_t;
struct sysqueues {

    //! Unrolls the `UEL_SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N` value to its power-of-two form
    #define UEL_SYSQUEUES_EVENT_QUEUE_SIZE (1<<UEL_SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N)
    //! The event queue buffer
    void *event_queue_buffer[UEL_SYSQUEUES_EVENT_QUEUE_SIZE];
    /** \brief The application's event queue.
      *
      * Holds events ready to be processed on the next runloop.
      */
    uel_cqueue_t event_queue;


    //! Unrolls the `UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N` value to its power-of-two form
    #define UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE (1<<UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N)
    //! The schedule queue buffer
    void *schedule_queue_buffer[UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE];
    /** \brief The application's schedule queue.
      *
      * Hold events already processed by the runloop but fit for rescheduling at
      * the scheduler.
      */
    uel_cqueue_t schedule_queue;
};

/** \brief Initialises a new uel_sysqueues_t
  *
  * \param queues The uel_sysqueues_t instance to be initialised
  */
void uel_sysqueues_init(uel_sysqueues_t *queues);

/** \brief Pushes an event into the event queue.
  *
  * This makes the event ready for colletion e processing by the event loop.
  *
  * \param queues The uel_sysqueues_t instance to be initialised
  * \param event The event to be enqueued
  */
void uel_sysqueues_enqueue_event(uel_sysqueues_t *queues, uel_event_t *event);

/** \brief Pops an event from the event queue.
  *
  * \param queues The uel_sysqueues_t instance from whose event queue the event must
  * be popped.
  * \returns The popped event's address. If the queue is empty, returns NULL.
  */
uel_event_t *uel_sysqueues_get_enqueued_event(uel_sysqueues_t *queues);

/** \brief Counts the number of elements in the event queue
  *
  * \param queues The uel_sysqueues_t instance whose event queue's elements should
  * be counted
  * \returns The number of elements in the queue
  */
uintptr_t uel_sysqueues_count_enqueued_events(uel_sysqueues_t *queues);

/** \brief Pushes an event into the schedule queue.
  *
  * This makes the event ready for collection and scheduling by the scheduler.
  *
  * \param queues The uel_sysqueues_t instance to be initialised
  * \param event The event to be scheduled
  */
void uel_sysqueues_schedule_event(uel_sysqueues_t *queues, uel_event_t *event);

/** \brief Pops an event from the schedule queue.
  *
  * \param queues The uel_sysqueues_t instance from whose schedule queue the event must
  * be popped.
  * \returns The popped event's address. If the queue is empty, returns NULL.
  */
uel_event_t *uel_sysqueues_get_scheduled_event(uel_sysqueues_t *queues);

/** \brief Counts the number of elements in the schedule queue
  *
  * \param queues The uel_sysqueues_t instance whose schedule queue's elements should
  * be counted
  * \returns The number of elements in the queue
  */
uintptr_t uel_sysqueues_count_scheduled_events(uel_sysqueues_t *queues);

#endif /* end of include guard: UEL_SYSTEM_QUEUES_H */
