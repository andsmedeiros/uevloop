/** \file config.h
  *
  * \brief Central repository of system configuration. This is meant to be edited by the
  * programmer as needed.
  */

#ifndef UEVLOOP_CONFIG_H
#define UEVLOOP_CONFIG_H

/* SYSPOOLS MODULE CONFIGURATION */

//! Defines the size of the event pool size in log2 form. Defaults to 128 events.
#define SYSPOOLS_EVENT_POOL_SIZE_LOG2N   (7)

//! Defines the size of the linked list node pool size in log2 form. Defaults to 128 nodes.
#define SYSPOOLS_LLIST_NODE_POOL_SIZE_LOG2N    (7)


/* SYSQUEUES MODULE CONFIGURATION */

//! The size of the event queue in log2 form. Defaults to 32 events.
#define SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N (5)

//! The size of the schedule queue in log2 form. Defaults to 32 events.
#define SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N (4)


/* SIGNAL MODULE CONFIGURATION */

//! \brief Defines the max number of listeners to be attached to an speciffic
//! signal in a single relay
#define SIGNAL_MAX_LISTENERS 5

#endif /* end of include guard: UEVLOOP_CONFIG_H */
