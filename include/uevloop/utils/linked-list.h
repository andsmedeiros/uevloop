/** \file linked-list.h
  *
  * \brief Defines a simple implementation of linked lists and functions to manipulate it
  */

#ifndef UEL_LINKED_LIST_H
#define	UEL_LINKED_LIST_H

/// \cond
#include <stdint.h>
#include <stdbool.h>
/// \endcond

#include "uevloop/utils/closure.h"

//! Defines a node of the linked list. Holds a void pointer.
typedef struct uel_llist_node uel_llist_node_t;
struct uel_llist_node{
    //! The value of the node, as a void pointer
    void *value;
    //! The next node in the list
    uel_llist_node_t *next;
};

/** \brief Defines a linked list. If it is empty, head == tail == NULL.
  * Pushing or popping from both the head or tail is always O(1).
  */
typedef struct uel_llist uel_llist_t;
struct uel_llist{
    //! A pointer to the head of the list. Is NULL when the list is empty.
    uel_llist_node_t *head;
    //! A pointer to the tail of the list. Is NULL when the list is empty.
    uel_llist_node_t *tail;
    //! The count of enqueued nodes
    uintptr_t count;
};

/** \brief Initialised a linked list
  *
  * \param list The list to be initialised. It will be empty after initialisation.
  */
void uel_llist_init(uel_llist_t *list);

/** \brief Pushes a node to the head of the list
  *
  * \param list The list into which to insert the node
  * \param node The node to be inserted.
  */
void uel_llist_push_head(uel_llist_t *list, uel_llist_node_t *node);

/** \brief Pushes a node to the tail of the list
*
* \param list The list into which to insert the node
* \param node The node to be inserted.
*/
void uel_llist_push_tail(uel_llist_t *list, uel_llist_node_t *node);

/** \brief Pops a node from the head of the list
*
* \param list The list from where the node will be popped
* \return node A pointer to the popped node if it exists. Otherwise, NULL.
*/
uel_llist_node_t *uel_llist_pop_head(uel_llist_t *list);

/** \brief Pops a node from the tail of the list
*
* \param list The list from where the node will be popped
* \return node A pointer to the popped node if it exists. Otherwise, NULL.
*/
uel_llist_node_t *uel_llist_pop_tail(uel_llist_t *list);

/** \brief Peeks the element at the head of the list
*
* \param list The list from where the node will be peeked
* \return node A pointer to the peeked node if it exists. Otherwise, NULL.
*/
uel_llist_node_t *uel_llist_peek_head(uel_llist_t *list);

/** \brief Peeks the element at the tail of the list
*
* \param list The list from where the node will be peeked
* \return node A pointer to the peeked node if it exists. Otherwise, NULL.
*/
uel_llist_node_t *uel_llist_peek_tail(uel_llist_t *list);

/** \brief Removes a node from the queue
  *
  * \param list The list from where the node will be removed
  * \param node Address of the node being removed
  * \returns Whether the node was found and removed
  */
bool uel_llist_remove(uel_llist_t *list, uel_llist_node_t *node);

/** \brief Splits a list in two. The rupture point is determined by the supplied
 * closure.
  *
  * This function iterates the linked list invoking the provided closure for each
  * node in it. While the closure returns true, nodes are popped from the list.
  *
  * \param list The list from where to pop nodes
  * \param should_remove A closure that will be invoked taking each node as parameter.
  * The closure should return a boolean indicating whether the node should be
  * removed or not.
  * \return A new list containing the removed nodes, in their original order.
  */
uel_llist_t uel_llist_remove_while(uel_llist_t *list, uel_closure_t *should_remove);

/** \brief Scans a list until it finds a suitable spot to insert the provided node.
  *
  * This function iterates the linked list invoking the supplied closure until
  * it returns true. The supplied node is then inserted at this position.
  *
  * The `should_insert` closure will be invoked for each pair of node addresses
  * in the range [NULL, &NODE_1, &NODE_2, ..., &NODE_N, NULL] as parameter.
  * When it finds a suitable position, it must return true.
  *
  * \param list The list to insert the node at
  * \param node The node to be inserted
  * \param should_insert A closure that returns whether the node ought to be
  * inserted at the current position. The closure function must unpack the
  * closure parameters to a `uel_llist_node_t **[2]` and return a boolean.
  */
void uel_llist_insert_at(uel_llist_t *list, uel_llist_node_t *node, uel_closure_t *should_insert);

#endif	/* UEL_LINKED_LIST_H */
