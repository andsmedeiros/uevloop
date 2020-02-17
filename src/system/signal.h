/** \file signal.h
  * \brief Defines signals and relays, structures used to communicate
  * asynchronously between distant parts of the application
  */

#ifndef SIGNAL_H
#define SIGNAL_H

#include "containers/system-pools.h"
#include "containers/system-queues.h"
#include "../utils/linked-list.h"
#include "../utils/closure.h"

/** \typedef signal_t
  *
  * Any signal is defined as an unsigned integer type. This means that any signal
  * type is only valid within the scope defined by the relay designed to respond
  * to said signal.
  */
typedef uintptr_t signal_t;

//! \brief Keeps a reference to a listener bound to some signal at some relay
typedef struct signal_listener signal_listener_t;
struct signal_listener{
    //! The list associated to the signal emitted when this listener was registered
    llist_t *source;
    //! The node containing this listener's closure to be run
    llist_node_t *node;
};

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
typedef struct signal_relay signal_relay_t;
struct signal_relay{
    //! Contains the signal vector. Must be large enough to contain every signal
    //! bound to this relay.
    llist_t *signal_vector;
    //! The system's internal queues. Upon emission, signals will be enqueued on
    //! one of these.
    sysqueues_t *queues;
    //! A quick reference to the system's internal pools
    syspools_t *pools;
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
void signal_relay_init(
    signal_relay_t *relay,
    syspools_t *pools,
    sysqueues_t *queues,
    llist_t *buffer,
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
signal_listener_t signal_listen(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
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
signal_listener_t signal_listen_once(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
);

/** \brief Detaches some listener from the signal vector at the provided relay.
  *
  + \param listener The listener that identifies the listen operation to be undone
  * \param relay The relay where the listener was registered
  */
void signal_unlisten(signal_listener_t listener, signal_relay_t *relay);

/** \brief Emits a signal at the supplied relay. Any closure listening to this
  * signal will be invoked.
  *
  * \param signal The signal to be emitted
  * \param relay The relay where the signal is registered
  * \param params The parameters supplied to the listener's closure when it is
  * invoked.
  */
void signal_emit(signal_t signal, signal_relay_t *relay, void *params);

#endif /* end of include guard: SIGNAL_H */
