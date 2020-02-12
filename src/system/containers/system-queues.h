/** \file system-queues.h
  * \brief A container for the system's internal queues
  */

#ifndef SYSTEM_QUEUES_H
#define SYSTEM_QUEUES_H

#include <stdint.h>
#include "../event.h"
#include "../../utils/circular-queue.h"

//! The size of the event queue in log2 form
#define SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N (5)
//! Unrolls the `SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N` value to its power-of-two form
#define SYSQUEUES_EVENT_QUEUE_SIZE (1<<SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N)

//! The size of the schedule queue in log2 form
#define SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N (4)
//! Unrolls the `SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N` value to its power-of-two form
#define SYSQUEUES_SCHEDULE_QUEUE_SIZE (1<<SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N)

typedef struct sysqueues sysqueues_t;
struct sysqueues {
    //! The event queue buffer
    void *event_queue_buffer[SYSQUEUES_EVENT_QUEUE_SIZE];
    /** \brief The application's event queue.
      *
      * Holds events ready to be processed on the next runloop.
      */
    cqueue_t event_queue;

    //! The schedule queue buffer
    void *schedule_queue_buffer[SYSQUEUES_SCHEDULE_QUEUE_SIZE];
    /** \brief The application's schedule queue.
      *
      * Hold events already processed by the runloop but fit for rescheduling at
      * the scheduler.
      */
    cqueue_t schedule_queue;
};

/** \brief Initialises a new sysqueues_t
  *
  * \param queues The sysqueues_t instance to be initialised
  */
void sysqueues_init(sysqueues_t *queues);

/** \brief Pushes an event into the event queue.
  *
  * This makes the event ready for colletion e processing by the event loop.
  *
  * \param queues The sysqueues_t instance to be initialised
  * \param event The event to be enqueued
  */
void sysqueues_enqueue_event(sysqueues_t *queues, event_t *event);

/** \brief Pops an event from the event queue.
  *
  * \param queues The sysqueues_t instance from whose event queue the event must
  * be popped.
  * \returns The popped event's address. If the queue is empty, returns NULL.
  */
event_t *sysqueues_get_enqueued_event(sysqueues_t *queues);

/** \brief Counts the number of elements in the event queue
  *
  * \params queues The sysqueues_t instance whose event queue's elements should
  * be counted
  * \returns The number of elements in the queue
  */
uintptr_t sysqueues_count_enqueued_events(sysqueues_t *queues);

/** \brief Pushes an event into the schedule queue.
  *
  * This makes the event ready for collection and scheduling by the scheduler.
  *
  * \param queues The sysqueues_t instance to be initialised
  * \param event The event to be scheduled
  */
void sysqueues_schedule_event(sysqueues_t *queues, event_t *event);

/** \brief Pops an event from the schedule queue.
  *
  * \param queues The sysqueues_t instance from whose schedule queue the event must
  * be popped.
  * \returns The popped event's address. If the queue is empty, returns NULL.
  */
event_t *sysqueues_get_scheduled_event(sysqueues_t *queues);

/** \brief Counts the number of elements in the schedule queue
  *
  * \params queues The sysqueues_t instance whose schedule queue's elements should
  * be counted
  * \returns The number of elements in the queue
  */
uintptr_t sysqueues_count_scheduled_events(sysqueues_t *queues);

#endif /* end of include guard: SYSTEM_QUEUES_H */
