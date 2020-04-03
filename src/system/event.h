/** \file event.h
  * \brief Defines events and methods to configure events.
  */

#ifndef UEL_EVENT_H
#define	UEL_EVENT_H

/// \cond
#include <stdint.h>
#include <stdbool.h>
/// \endcond

#include "../utils/closure.h"
#include "../utils/linked-list.h"

//! Possible types of events understood by the core
enum uel_event_type {
    UEL_CLOSURE_EVENT,
    UEL_TIMER_EVENT,
    UEL_SIGNAL_EVENT,
    UEL_SIGNAL_LISTENER_EVENT
};
//! Alias to the uel_event_type enum.
typedef enum uel_event_type uel_event_type_t;


//! Possible statuses for a timer event
enum uel_event_timer_status {
    UEL_TIMER_RUNNING,
    UEL_TIMER_PAUSED,
    UEL_TIMER_CANCELLED
};
//! Alias to the uel_event_timer_status
typedef enum uel_event_timer_status uel_event_timer_status_t;


/** \brief Events are special messages passed around the core.
  * They represent tasks to be run at some point by the system.
  *
  * Events are bound to information on how and when they should be invoked.
  * There are four types of events:
  *
  * - `UEL_CLOSURE_EVENT`: lifeless wrappers to closures.
  * - `UEL_TIMER_EVENT`: contains scheduling information associated with some closure
  * - `UEL_SIGNAL_EVENT`: contains information on the emission of a signal
  * - `UEL_SIGNAL_LISTENER_EVENT`: represent a single listening operation
  *
  * Closure and timer events can be recurring, in which case they won't be discarded
  * after processing by the event loop.
  *
  * Signal events are *always* discarded by the event loop.
  * Signal listeners are never meant to be sent there. In their case, the `repeating`
  * flag determines whether the signal should be able to fire multiple times or just once.
  */
typedef struct event uel_event_t;
struct event {
    uel_event_type_t type; //!< The type of the event, as defined by `uel_event_type_t`
    uel_closure_t closure; //!< The closure to be invoked a.k.a. the action to be run
    /** \brief Marks whether the event should be discarded after processing.
      * Closure events have no use for this flag
      */
    bool repeating;
    //! Allows to compact many speciffic details on various event types on a single
    //! memory slot. Pertinent content depends on the `type` member value.
    union detail{
        //! Contains information suitable for scheduling an event at the scheduler.
        struct timer{
            /** \brief The value the system timer must be at when this event's closure
            * should be invoked. This is a best effort value.
            */
            uint32_t due_time;
            uint16_t timeout; //!< Holds the interval between two executions of the timer
            uel_event_timer_status_t status; //!< Current timer status
        } timer; //!< The scheduling information of this event. Relevant only for timers
        //! Contains information related to an emitted `signal`.
        struct signal{
            uintptr_t value; //!< The integer value that identifies this signal
            uel_llist_t *listeners; //!< Reference to the signal listeners
        } signal; //!< The emission information of this event. Relevant only for signals
        //! Contains the context of a particular signal listener
        struct listener{
            /** When this flag is set, the `event_loop` will not run this event's
              * closure. Additionally, the event will be destroyed.
              */
            bool unlistened;
        } listener; //!< The listening information of this event. Relevant only for signal listeners
    } detail; //!< Represents speciffic detail on a event depending on its type.
};

/** \brief Destroys an event
  *
  * At the moment, all this does is invoke `uel_closure_destroy` in the contained closure
  * \param event The event to be destroyed
  */
void uel_event_destroy(uel_event_t *event);

/** \brief Configures a closure event
  *
  * \param event The event to be configured
  * \param closure The closure to be invoked when the event is run
  * \param repeating Intructs the system whether should this event be disposed of
  * after processing
  */
void uel_event_config_closure(uel_event_t *event, uel_closure_t *closure, bool repeating);

/** \brief Configures a signal event
  *
  * \param event The event to be configured
  * \param signal The integer value that identifies this signal
  * \param listeners The listeners associated to this signal
  * \param params The parameters associated with this signal emission
  */
void uel_event_config_signal(
    uel_event_t *event,
    uintptr_t signal,
    uel_llist_t *listeners,
    void *params
);

/** \brief Configures a signal listener event
  *
  * \param event The event to be configured
  * \param closure The closure to be invoked when the event is run
  * \param repeating Intructs the system whether should this event be disposed of
  * after processing
  */
void uel_event_config_signal_listener(uel_event_t *event, uel_closure_t *closure, bool repeating);

/** \brief Configures a timer event
  * \param event The event to be configured
  * \param timeout_in_ms The delay to process this event. If the event is repeating,
  * this defines the interval between successive executions.
  * \param repeating If this flag is set, the event will not be destroyed after
  * execution. Instead it will be put on the schedule queue.
  * \param immediate If this flag is set, a recurring timer will be immediately
  * enqueued for processing. One-shot timers will ignore this setting.
  * \param closure The closure to be invoked when the timer expires
  * \param current_time The current value set in the system counter.
  */
void uel_event_config_timer(
    uel_event_t *event,
    uint16_t timeout_in_ms,
    bool repeating,
    bool immediate,
    uel_closure_t *closure,
    uint32_t current_time
);

/** \brief Pauses a timer event
  *
  * \param event The timer event to be paused
  */
void uel_event_timer_pause(uel_event_t *event);

/** \brief Resumes a paused timer event
  *
  * \param event The timer event to be resumed
  */
void uel_event_timer_resume(uel_event_t *event);

/** \brief Cancels a timer event
  *
  * \param event The timer event to be cancelled
  */
void uel_event_timer_cancel(uel_event_t *event);

#endif	/* UEL_EVENT_H */
