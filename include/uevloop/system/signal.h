/** \file signal.h
  * \brief Defines signals and relays, structures used to communicate
  * asynchronously between distant parts of the application
  */

#ifndef UEL_SIGNAL_H
#define UEL_SIGNAL_H

#include "uevloop/utils/linked-list.h"
#include "uevloop/utils/closure.h"
#include "uevloop/utils/promise.h"
#include "uevloop/system/containers/system-pools.h"
#include "uevloop/system/containers/system-queues.h"
#include "uevloop/system/event.h"

/** \typedef uel_signal_t
  *
  * Any signal is defined as an unsigned integer type. This means that any signal
  * type is only valid within the scope defined by the relay designed to respond
  * to said signal.
  */
typedef uintptr_t uel_signal_t;

/** \brief Defines a reference type suitable to manipulate the actual listener
  * event's context.
  */
typedef struct listener *uel_signal_listener_t;

/** \brief Contains a signal vector and operates on in.
  *
  * The signal relay is the central data structure involved in signal operation.
  * It contains a signal vector, an array of linked lists, each associated to a
  * particular signal.
  *
  * When a signal is listened for, the listener closure is added to the linked
  * list corresponding to said signal. When that signal is emitted, each listener
  * closure in the list is invoked.
  */
typedef struct uel_signal_relay uel_signal_relay_t;
struct uel_signal_relay{
    //! Contains the signal vector. Must be large enough to contain every signal
    //! bound to this relay.
    uel_llist_t *signal_vector;
    //! The system's internal queues. Upon emission, signals will be enqueued on
    //! one of these.
    uel_sysqueues_t *queues;
    //! A quick reference to the system's internal pools
    uel_syspools_t *pools;
    //! The number of signals registered at this relay.
    uintptr_t width;
};

/** \brief Initialises a signal relay
  *
  * \param relay The signal relay object to be initialised
  * \param pools The system's internal pools
  * \param queues The system's internal queues
  * \param buffer The buffer that will be used to store listeners registered at
  * this relay. Must be the number of signals bound to this relay wide.
  * \param width The number of signals bound to this relay.
  */
void uel_signal_relay_init(
    uel_signal_relay_t *relay,
    uel_syspools_t *pools,
    uel_sysqueues_t *queues,
    uel_llist_t *buffer,
    uintptr_t width
);

/** \brief Attaches a listener closure to some signal at a particular relay
  *
  * \param signal The signal to be listened for
  * \param relay The relay where the listener will be registered
  * \param closure The closure to be invoked when the signal is emitted. The
  * closure will be invoked with whatever parameters are supplied during emission.
  * \return Returns a listener that references this particular operation
  */
uel_signal_listener_t uel_signal_listen(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_closure_t *closure
);

/** \brief Attaches a listener closure to some signal at a particular relay. The
  * listener closure will be invoked only once.
  *
  * \param signal The signal to be listened for
  * \param relay The relay where the listener will be registered
  * \param closure The closure to be invoked when the signal is emitted. The
  * closure will be invoked with whatever parameters are supplied during emission.
  * \return Returns a listener that references this particular operation
  */
uel_signal_listener_t uel_signal_listen_once(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_closure_t *closure
);

/** \brief Marks a signal listener as expired. When its corresponding signal is
  * emitted, this listener's closure will not be invoked and the listener will
  * be destroyed.
  *
  + \param listener The listener that identifies the listen operation to be undone
  */
void uel_signal_unlisten(uel_signal_listener_t listener);

/** \brief Emits a signal at the supplied relay. Any closure listening to this
  * signal will be asynchronously invoked.
  *
  * \param signal The signal to be emitted
  * \param relay The relay where the signal is registered
  * \param params The parameters supplied to the listener's closure when it is
  * invoked.
  */
void uel_signal_emit(uel_signal_t signal, uel_signal_relay_t *relay, void *params);

/** \brief Attaches a non-repeating listener that resolves the provided promise
  * upon emission.
  *
  * \param signal The signal to be listened for
  * \param relay The relay where the signal is registered
  * \param promise The promise to be resolved upon signal emission
  * \returns The listener associated with this operation
  */
uel_signal_listener_t uel_signal_resolve_promise(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_promise_t *promise
);

/** \brief Attaches a non-repeating listener that rejects the provided promise
  * upon emission.
  *
  * \param signal The signal to be listened for
  * \param relay The relay where the signal is registered
  * \param promise The promise to be rejected upon signal emission
  * \returns The listener associated with this operation
  */
uel_signal_listener_t uel_signal_reject_promise(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_promise_t *promise
);

#endif /* end of include guard: UEL_SIGNAL_H */
