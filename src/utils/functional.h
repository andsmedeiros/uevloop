/** \file functional.h
  *
  * \brief Contains helpers for composing and augumenting closures
  */

#ifndef UEL_FUNCTIONAL_H
#define UEL_FUNCTIONAL_H

/// \cond
#include <stdlib.h>
/// \endcond

#include "closure.h"
#include "pipeline.h"
#include "conditional.h"
#include "iterator.h"

/** \brief Maps elements of an iterator to an area of memory. Each element is
  * assigned to a `void` pointer slot.
  */
typedef struct uel_func_mapper uel_func_mapper_t;
struct uel_func_mapper{
    //! The iterator that enumerates the elements to be mapped
    uel_iterator_t *iterator;
    //! The area of memory to where elements will be mapped
    void **destination;
    //! The maximum size of slots to be mapped to
    size_t limit;
};

/** \brief Initialises a mapper object
  *
  * \param mapper The mapper instance to be initialised
  * \param iterator The iterator responsible for enumerating elements
  * \param destination An array of `void` pointers to where elements will be mapped
  * \param limit The maximum size of elements that can be mapped
  */
void uel_func_mapper_init(uel_func_mapper_t *mapper, uel_iterator_t *iterator,
                                            void **destination, size_t limit);

/** \brief Wraps a pipeline in a closure
  *
  * This function wraps a pipeline in a closure. When invoked, the closure's
  * parameters will be forwarded to the pipeline. Also, the pipeline's returned
  * value will be returned from the closure.
  *
  * \param pipeline The pipeline to be wrapped
  * \returns A closure that, when invoked, will apply the supplied pipeline
  */
uel_closure_t uel_func_pipeline(uel_pipeline_t *pipeline);

/** \brief Wraps a conditional in a closure
  *
  * This function wraps a conditional in a closure. When invoked, the closure's
  * parameters will be forwarded to the conditional. Also, the conditional's
  * returned value will be returned from the closure.
  *
  * \param conditional The conditional to be wrapped
  * \returns A closure that, when invoked, will apply the supplied conditional
  */
uel_closure_t uel_func_conditional(uel_conditional_t *conditional);

/** \brief Wraps a closure in a `foreach` construct
  *
  * Creates a new closure based on a supplied closure. When invoked, this new
  * closure takes an  `uel_iterator_t` for parameter and yields each element in
  * it to the supplied  closure.
  *
  * See `uel_iterator_foreach()`.
  *
  * \param closure A reference to the closure to be wrapped
  */
uel_closure_t uel_func_foreach(uel_closure_t *closure);

/** \brief Wraps a closure in a `map` construct
  *
  * Creates a new closure based on a supplied closure. This new closure takes an
  * `uel_func_mapper_t` for parameter describing the mapping relation between
  * elements yielded from an iterator and a region of memory.
  *
  * When invoked, this new closure passes each element enumerated by this mapper
  * as parameter to the supplied closure and stores its returned value sequentially
  * in the destination area. It then returns the destination address.
  *
  * See `uel_iterator_map()`.
  *
  * \param closure A reference to the closure to be wrapped
  */
uel_closure_t uel_func_map(uel_closure_t *closure);

/** \brief Wraps a closure in a `find` construct
  *
  * Creates a new closure based on a supplied closure. This new closure accepts
  * an `uel_iterator_t` as parameter and applies the supplied closure to each
  * element yielded by this iterator.
  *
  * When some enumerated element causes the supplied closure to return true, its
  * address is returned by the new closure.
  *
  * \param closure The test to be applied against each enumerated element
  * \returns The address of the first element that passes the supplied test. If
  * no such element is found, returns `NULL`.
  */
uel_closure_t uel_func_find(uel_closure_t *closure);

/** \brief Wraps a closure in a `count` construct
  *
  * Creates a new closure based on a supplied closure. This new closure accepts
  * an `uel_iterator_t` as parameter and applies the supplied closure to each
  * element yielded by this iterator.
  *
  * This new closure counts the number of enumerated elements that caused the
  * test closure to return `true`.
  *
  * \param closure The test to be applied against each enumerated element
  * \returns The number of elements that passed the supplied test closure.
  */
uel_closure_t uel_func_count(uel_closure_t *closure);

/** \brief Wraps a closure in a `all` construct
  *
  * Creates a new closure based on a supplied closure. This new closure accepts
  * an `uel_iterator_t` as parameter and applies the supplied closure to each
  * element yielded by this iterator.
  *
  * If all enumerate elements cause the supplied closure to return `true`, this
  * new closure also returns `true`. Otherwise, it returns `false`.
  *
  * \param closure The test to be applied against each enumerated element
  * \returns Whether all elements pass the supplied test closure.
  */
uel_closure_t uel_func_all(uel_closure_t *closure);

/** \brief Wraps a closure in a `none` construct
  *
  * Creates a new closure based on a supplied closure. This new closure accepts
  * an `uel_iterator_t` as parameter and applies the supplied closure to each
  * element yielded by this iterator.
  *
  * If all enumerate elements cause the supplied closure to return `false`, this
  * new closure also returns `true`. Otherwise, it returns `false`.
  *
  * \param closure The test to be applied against each enumerated element
  * \returns Whether all elements are reproved in the supplied test closure.
  */
uel_closure_t uel_func_none(uel_closure_t *closure);

/** \brief Wraps a closure in a all construct
  *
  * Creates a new closure based on a supplied closure. This new closure accepts
  * an `uel_iterator_t` as parameter and applies the supplied closure to each
  * element yielded by this iterator.
  *
  * If any enumerate elements cause the supplied closure to return `true`, this
  * new closure also returns `true`. Otherwise, it returns `false`.
  *
  * \param closure The test to be applied against each enumerated element
  * \returns Whether any elements pass the supplied test closure.
  */
uel_closure_t uel_func_any(uel_closure_t *closure);

#endif /* end of include guard: UEL_FUNCTIONAL_H */
