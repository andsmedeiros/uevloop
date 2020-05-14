/** \file iterator.h
  *
  * \brief Defines iterators, structures and functions suitable for enumerating
  * other data structures.
  */

#ifndef UEL_ITERATOR_H
#define UEL_ITERATOR_H

/// \cond
#include <stdlib.h>
#include <limits.h>
/// \endcond

#include "uevloop/utils/linked-list.h"
#include "uevloop/utils/closure.h"

/** \brief Iterators are data structures that wrap arbitrary collections of data
  * and define ways of enumerating them.
  *
  * This is an unspecialised structure that should be augumented with actual
  * behaviour implementation.
  *
  * In order to specialise it, create a struct that holds whatever state is
  * meaningful to iteration and has a `uel_iterator_t` as its first member.
  * Then, whenever needed, cast the iterator to your custom type.
  */
typedef struct uel_iterator uel_iterator_t;
struct uel_iterator{
    /** \brief The iteration function assigned to this iterator type
      *
      * This is the actual implementation of the iteration algorithm. It takes a
      * reference to the iterator and the last accessed element's address and
      * returns a reference to the next element.
      *
      * To resquest the first collection item, pass NULL as the last element
      * parameter.
      * When this function returns NULL, there are no more elements in the
      * collection to be iterated over.
      */
    void * (* next)(uel_iterator_t *, void *);
    //! A reference to the collection being iterated.
    void *collection;
};

/** \brief Applies a closure to an enumerable collection
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The closure to be invoked with each element in the enumerated
  * collection as parameter. If this closure returns `false`, the iteration will
  * be halted.
  * \returns Whether the collection was fully iterated over or if the process was
  * halted by returning `false`.
  */
bool uel_iterator_foreach(uel_iterator_t *iterator, uel_closure_t *closure);

/** \brief Applies a closure to an enumerable collection and stores its results
  *
  * This function takes an iterator that enumerates elements in a collection and
  * yields each one to a closure. The return value of each closure call is stored
  * in an array.
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The closure to be invoked with each element in the enumerated
  * collection ad parameter.
  * \param destination The destination of the data produced by the invoked closure.
  * This must be an array of void pointers large enough to store all produced data
  * \param limit The maximum size of the destination buffer. If collection is
  * larger than this array, in elements, it will not be iterated entirely, only
  * up to the `limit`th element.
  * \returns The number of effectively iterated elements.
  */
size_t uel_iterator_map(uel_iterator_t *iterator, uel_closure_t *closure, void **destination, size_t limit);

/** \brief Finds the first element enumerated by an iterator that passes the
  * supplied test
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The test to be applied against each enumerated element
  * \returns The address of the first element that passes the supplied test. If
  * no such element is found, returns NULL.
  */
void *uel_iterator_find(uel_iterator_t *iterator, uel_closure_t *closure);

/** \brief Counts elements enumerated by an iterator that pass the supplied test
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The test to be applied against each enumerated element
  * \returns How many elements successfully passed the supplied test
  */
size_t uel_iterator_count(uel_iterator_t *iterator, uel_closure_t *closure);

/** \brief Determines whether all elements enumerated by an iterator pass the
  * supplied test or not.
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The test to be applied against each enumerated element
  * \returns Whether all elements are approved by the test closure.
  */
bool uel_iterator_all(uel_iterator_t *iterator, uel_closure_t *closure);

/** \brief Determines whether all elements enumerated by an iterator fail the
  * supplied test or not.
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The test to be applied against each enumerated element
  * \returns Whether all elements are reproved by the test closure.
  */
bool uel_iterator_none(uel_iterator_t *iterator, uel_closure_t *closure);

/** \brief Determines whether any elements enumerated by an iterator pass the
  * supplied test or not.
  *
  * \param iterator The iterator that enumerates the elements of some collection
  * \param closure The test to be applied against each enumerated element
  * \returns Whether any elements are approved by the test closure. If such
  * element is found, this function returns early.
  */
bool uel_iterator_any(uel_iterator_t *iterator, uel_closure_t *closure);

/** \brief A specialised iterator suitable for iterating over arrays of arbitrary
  * data
  */
typedef struct uel_iterator_array uel_iterator_array_t;
struct uel_iterator_array{
    //! The base iterator interface
    uel_iterator_t base;
    //! The number of elements in the array
    size_t item_count;
    //! The size of each element in the array
    size_t item_size;
};

/** \brief Yields elements in a array
  *
  * \param iterator The iterator thar enumerates the elements in the target array
  * \param last The last element yielded. If `NULL`, returns the first element.
  * \returns The next element in the array with respect to the last one provided.
  * If there are no more elements in the array, this returns `NULL`.
  */
void *uel_iterator_array_next(uel_iterator_array_t *iterator, void *last);

/** \brief Creates a new array iterator
  *
  * \param collection The array to be  enumerated
  * \param count The number of elements in this array
  * \param size The size of each element in the array
  * \returns The created iterator
  */
uel_iterator_array_t uel_iterator_array_create(void *collection, size_t count, size_t size);

/** \brief Alias to `uel_iterator_t`. Iterator suitable to traverse linked lists.
  *
  * Because when traversing a linked list we need only know one element to know
  * the next one, there is no need to store any additional state in linked list
  * iterators.
  */
typedef uel_iterator_t uel_iterator_llist_t;

/** \brief Yields elements in a linked list
  *
  * \param iterator The iterator thar enumerates the elements in the target list
  * \param last The last element yielded. If `NULL`, returns the first element.
  * \returns The next element in the list with respect to the last one provided.
  * If there are no more elements in the list, this returns `NULL`.
  */
void *uel_iterator_llist_next(uel_iterator_t *iterator, void *last);

/** \brief Creates a new linked list iterator
  *
  * \param list The linked list to be enumerated
  * \returns The created iterator
  */
uel_iterator_llist_t uel_iterator_llist_create(uel_llist_t *list);

//! Defines the maximum possible iteration limit for the `uel_iterator_map()`
//! function.
#define UEL_ITERATOR_MAP_BOUNDLESS UINT_MAX

#endif /* UEL_ITERATOR_H */
