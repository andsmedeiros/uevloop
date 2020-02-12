/** \file system-pools.h
  * \brief A container for the system's internal pools
  */

#ifndef SYSTEM_POOLS_H
#define	SYSTEM_POOLS_H

#include <stdint.h>
#include "../event.h"
#include "../../utils/object-pool.h"
#include "../../utils/linked-list.h"

/** \brief A container for the system pools
  *
  * The syspools object is meant as a container for the internal system pools.
  * It is defined to make easier for the programmer to reason about the sizes
  * of allocated internal object pools.
  */
typedef struct syspools syspools_t;
struct syspools{

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
  * \param pools The syspools_t instance
  */
void syspools_init(syspools_t *pools);

/** \brief Acquires an event from the system pools
  *
  * \param pools The syspools_t instance
  * \returns The acquired event
  */
event_t *syspools_acquire_event(syspools_t *pools);

/** \brief Acquires a linked list node from the system pools
  *
  * \param pools The syspools_t instance
  * \returns The acquired linked list node
  */
llist_node_t *syspools_acquire_llist_node(syspools_t *pools);

/** \brief Releases an event to the system pools
  *
  * \param pools The syspools_t instance
  * \param event The event to be released
  * \returns Whether the event was successfully released
  */
bool syspools_release_event(syspools_t *pools, event_t *event);

/** \brief Releases a linked list node to the system pools
  *
  * \param pools The syspools_t instance
  * \param node The linked list node to be released
  * \returns Wheter the linked list node was successfully released
  */
bool syspools_release_llist_node(syspools_t *pools, llist_node_t *node);

#endif	/* SYSTEM_POOLS_H */
