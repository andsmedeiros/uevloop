/** \file event-loop.h
  * \brief Contains the evloop_t struct and functions to manipulate it
  */

#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "../utils/closure.h"
#include "../system/containers/system-pools.h"
#include "../system/containers/system-queues.h"

/** \brief The event loop object
  *
  * This object represents an event loop. It is operated primarily by the system
  * internal queues:
  * 1. The inbound event queue, which is the feeding point of the event loop.
  * 2. The outbound schedule queue, which holds reusable timers already run.
  */
typedef struct evloop evloop_t;
struct evloop{
    syspools_t *pools; //!< Reference to the system's pools
    sysqueues_t *queues; //!< Reference to the system's queues
};

/** \brief Initialises an event loop
  *
  * \param event_loop The evloop_t instance to be initialised
  * \param pools The system's internal pools
  * \param queues The system's internal queues
  */
void evloop_init(
    evloop_t *event_loop,
    syspools_t *pools,
    sysqueues_t *queues
);

/** \brief Triggers a runloop.
  *
  * This function flushes the event queue and processes each event in it.
  * Afterwards, depending on the event type, it disposes of the event in
  * different ways.
  *
  * Each iteration of this cycle is called a runloop.
  *
  * \param event_loop The evloop_t instance to be run
  */
void evloop_run(evloop_t *event_loop);

/** \brief Enqueues a closure to be invoked
  *
  * \param event_loop The evloop_t instance into which the closure will be enqueued
  * \param closure The closure to be enqueued
  */
void evloop_enqueue_closure(evloop_t *event_loop, closure_t *closure);

#endif /* end of include guard: EVENT_LOOP_H */
