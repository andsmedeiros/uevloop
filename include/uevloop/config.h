/** \file config.h
  *
  * \brief Central repository of system configuration. This is meant to be edited by the
  * programmer as needed.
  */

#ifndef UEL_CONFIG_H
#define UEL_CONFIG_H

/* UEL_SYSPOOLS MODULE CONFIGURATION */

#ifndef UEL_SYSPOOLS_EVENT_POOL_SIZE_LOG2N
//! Defines the size of the event pool size in log2 form. Defaults to 128 events.
#define UEL_SYSPOOLS_EVENT_POOL_SIZE_LOG2N   (7)
#endif /* UEL_SYSPOOLS_EVENT_POOL_SIZE_LOG2N */

#ifndef UEL_SYSPOOLS_LLIST_NODE_POOL_SIZE_LOG2N
//! Defines the size of the linked list node pool size in log2 form. Defaults to 128 nodes.
#define UEL_SYSPOOLS_LLIST_NODE_POOL_SIZE_LOG2N    (7)
#endif /* UEL_SYSPOOLS_LLIST_NODE_POOL_SIZE_LOG2N */


/* UEL_SYSQUEUES MODULE CONFIGURATION */

#ifndef UEL_SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N
//! The size of the event queue in log2 form. Defaults to 32 events.
#define UEL_SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N (5)
#endif /* UEL_SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N */

#ifndef UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N
//! The size of the schedule queue in log2 form. Defaults to 32 events.
#define UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N (4)
#endif /* UEL_SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N */


/* SIGNAL MODULE CONFIGURATION */

#ifndef UEL_SIGNAL_MAX_LISTENERS
//! \brief Defines the max number of listeners to be attached to an speciffic
//! signal in a single relay
#define UEL_SIGNAL_MAX_LISTENERS    (5)
#endif /* UEL_SIGNAL_MAX_LISTENERS */

/* PROMISE MODULE CONFIGURATION */

#ifndef UEL_PROMISE_POOL_SIZE_LOG2N
//! The size of the promise pool in log2 form. Default to 32 promises.
#define UEL_PROMISE_POOL_SIZE_LOG2N (5)
#endif /* UEL_PROMISE_POOL_SIZE_LOG2N */

#ifndef UEL_PROMISE_SEGMENT_POOL_SIZE_LOG2N
//! The size of the segment pool in log2 form. Defaults to 128 segments (4 x promises)
#define UEL_PROMISE_SEGMENT_POOL_SIZE_LOG2N (UEL_PROMISE_POOL_SIZE_LOG2N + 2)
#endif /* UEL_PROMISE_SEGMENT_POOL_SIZE_LOG2N */

//! Enable promise chain functions aliases: THEN, CATCH, AFTER, ALWAYS

#endif /* end of include guard: UEL_CONFIG_H */
