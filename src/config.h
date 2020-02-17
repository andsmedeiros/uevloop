#ifndef UEVLOOP_CONFIG_H
#define UEVLOOP_CONFIG_H

/* SYSPOOLS MODULE CONFIGURATION */

//! Defines the size of the event pool size in log2 form
#define SYSPOOLS_EVENT_POOL_SIZE_LOG2N   (7) //128 events

//! Defines the size of the linked list node pool size in log2 form
#define SYSPOOLS_LLIST_NODE_POOL_SIZE_LOG2N    (7) //128 nodes


/* SYSQUEUES MODULE CONFIGURATION */

//! The size of the event queue in log2 form
#define SYSQUEUES_EVENT_QUEUE_SIZE_LOG2N (5) // 32 events

//! The size of the schedule queue in log2 form
#define SYSQUEUES_SCHEDULE_QUEUE_SIZE_LOG2N (4) // 16 events


/* SIGNAL MODULE CONFIGURATION */

#define SIGNAL_MAX_LISTENERS 5

#endif /* end of include guard: UEVLOOP_CONFIG_H */
