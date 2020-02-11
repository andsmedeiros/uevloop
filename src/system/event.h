/** \file event.h
  * \brief Defines events and methods to configure events.
  */

#ifndef EVENT_H
#define	EVENT_H

#include <stdint.h>
#include <stdbool.h>
#include "../utils/closure.h"

//! Possible types of events understood by the core
enum event_type {
    CLOSURE_EVENT,
    TIMER_EVENT,
    SIGNAL_EVENT
};
//! Alias to the event_type enum.
typedef enum event_type event_type_t;

/** \brief Events are special closures passed around the core.
  * They represent tasks to be run at some point by the system.
  *
  * Events are bound to information on how and when they should be invoked.
  * They can be timers, which are scheduled to be run in the future, or signals,
  * which represent asynchronous reactions to occurances thorough the system.
  *
  * Both timers and signals can be recurring, in which case they won't be
  * discarded after processing by the event loop.
  *
  * Events can also be lifeless shells to closures, in which case they will be
  * immediately destroyed after execution.
  */
typedef struct event event_t;
struct event {
    event_type_t type; //!< The type of the event, as defined in event.h
    closure_t closure; //!< The closure to be invoked a.k.a. the action to be run
    /** \brief Marks whether the event should be discarded after processing.
      * Closure events have no use for this flag
      */
    bool repeating;
    struct{
        /** \brief The value the system timer must be at when this event's closure
          * should be invoked. This is a best effort value.
          */
        uint32_t due_time;
        uint16_t timeout; //!< Holds the interval between two executions of the timer
    } timer; //!< The scheduling information of this event. Relevant only for timers
};

/** \brief Destroys an event
  *
  * At the moment, all this does is invoke `closure_destroy` in the contained closure
  * \param event The event to be destroyed
  */
void event_destroy(event_t *event);

/** \brief Configures a closure event
  *
  * \param event The event to be configured
  * \param closure The closure to be invoked when the event is run
  */
void event_config_closure(event_t *event, closure_t *closure);

/** \brief Configures a signal event
  *
  * \param event The event to be configured
  * \param repeating If this flag is set, the event will not be destroyed after
  * execution.
  * \param closure The closure to be invoked when the event is run
  */
void event_config_signal(event_t *event, bool repeating, closure_t *closure);

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
void event_config_timer(
    event_t *event,
    uint16_t timeout_in_ms,
    bool repeating,
    bool immediate,
    closure_t *closure,
    uint32_t current_time
);

#endif	/* EVENT_H */
