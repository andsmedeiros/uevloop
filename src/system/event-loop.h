/** \file event-loop.h
  * \brief Module responsible for running enqueued events and process them accordingly
  */

#ifndef UEL_EVENT_LOOP_H
#define UEL_EVENT_LOOP_H

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
typedef struct uel_evloop uel_evloop_t;
struct uel_evloop{
    uel_syspools_t *pools; //!< Reference to the system's pools
    uel_sysqueues_t *queues; //!< Reference to the system's queues
};

/** \brief Initialises an event loop
  *
  * \param event_loop The uel_evloop_t instance to be initialised
  * \param pools The system's internal pools
  * \param queues The system's internal queues
  */
void uel_evloop_init(
    uel_evloop_t *event_loop,
    uel_syspools_t *pools,
    uel_sysqueues_t *queues
);

/** \brief Triggers a runloop.
  *
  * This function flushes the event queue and processes each event in it.
  * Afterwards, depending on the event type, it disposes of the event in
  * different ways.
  *
  * Each iteration of this cycle is called a runloop.
  *
  * \param event_loop The uel_evloop_t instance to be run
  */
void uel_evloop_run(uel_evloop_t *event_loop);

/** \brief Enqueues a closure to be invoked
  *
  * \param event_loop The uel_evloop_t instance into which the closure will be enqueued
  * \param closure The closure to be enqueued
  */
void uel_evloop_enqueue_closure(uel_evloop_t *event_loop, uel_closure_t *closure);

#endif /* end of include guard: UEL_EVENT_LOOP_H */
