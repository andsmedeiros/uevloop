/** \file pools.h
  * \brief A container for the system's internal pools
  */

#ifndef POOLS_H
#define	POOLS_H

#include <stdint.h>
#include "event.h"
#include "../utils/object-pool.h"
#include "../utils/linked-list.h"

/** \brief A container for the system pools
  *
  * The pools object is meant as a container for the internal system pools.
  * It is defined to make easier for the programmer to reason about the sizes
  * of allocated internal object pools
  */
typedef struct pools pools_t;
struct pools{

    //! Defines the size of the event pool size in log2 form
    #define EVENT_POOL_SIZE_LOG2N   (7) //128 events
    //! The buffer used to store events in the event pool
    uint8_t event_pool_buffer[(1<<EVENT_POOL_SIZE_LOG2N) * sizeof(event_t)];
    //! The buffer used to store event pointers in the event pool queue
    void *event_pool_queue_buffer[1<<EVENT_POOL_SIZE_LOG2N];
    //! The event pool object. Contains all the events used by the core.
    objpool_t event_pool;

    //! Defines the size of the linked list node pool size in log2 form
    #define LLIST_NODE_POOL_SIZE_LOG2N    (7) //128 nodes
    //! The buffer used to store llist nodes in the llist node pool
    uint8_t llist_node_pool_buffer[(1<<LLIST_NODE_POOL_SIZE_LOG2N) * sizeof(llist_node_t)];
    //! The budder used to store llist node pointers in the llist node pool queue
    void *llist_node_pool_queue_buffer[1<<LLIST_NODE_POOL_SIZE_LOG2N];
    //! The llist node pool object. Contains all llist nodes used by the core.
    objpool_t llist_node_pool;

};

/** \brief Initialise the system pools
  *
  * \param pools The pools_t instance
  */
void pools_init(pools_t *pools);

#endif	/* POOLS_H */
