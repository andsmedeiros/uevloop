/** \file pipeline.h
  *
  * \brief Defines pipelines, structures to hold many closures and functions to invoke
  * them sequentially, composing their functionality
  */

#ifndef UEL_PIPELINE_H
#define UEL_PIPELINE_H

/// \cond
#include <stdlib.h>
/// \endcond

#include "uevloop/utils/closure.h"

/** \brief A pipeline is an ordered closure list to be run in sequence.
  *
  * Pipelines contain an ordered list of closures. When applied to some initial
  * parameter, each closure is invoked with the previous one's return value as
  * parameter.
  */
typedef struct uel_pipeline uel_pipeline_t;
struct uel_pipeline{
    //! A reference to an array of closures to be run.
    uel_closure_t *closures;
    //! The number of closures in the array.
    size_t count;
};

/** \brief Initialises a new pipeline
  *
  * \param pipeline The pipeline to be initialised
  * \param closures The closures that should be run in sequence, in an array.
  * \param count The number of closures to be piped.
  */
void uel_pipeline_init(uel_pipeline_t *pipeline, uel_closure_t *closures, size_t count);

/** \brief Applies a pipeline to some input
  *
  * This function takes some input and passes it to the first closure
  * in the pipeline. The value returned from this first closure will be passed
  * to the second one and so on.
  *
  * \param pipeline The pipeline to be applied
  * \param params The initial parameters that will be passed to the first closure
  * \returns Whatever was returned by the last closure invoked
  */
void *uel_pipeline_apply(uel_pipeline_t *pipeline, void *params);

/** \brief Helper macro to create a pipeline and its required data structures
  *
  * This macro, based on the input identifier, declares:
  * - a closure array with any supplied closures ([id]_pipeline_closures)
  * - a pipeline object wrapping the closure array ([id]_pipeline)
  *
  * \param id The identifier on which to base declared variables' names.
  * \param ... The closures to be pipelined.
  */
#define UEL_PIPELINE_DECLARE(id, ...)                                         \
    uel_closure_t id##_pipeline_closures[] = {__VA_ARGS__};                   \
    uel_pipeline_t id##_pipeline;                                             \
    uel_pipeline_init(&id##_pipeline, id##_pipeline_closures,                 \
                    sizeof(id##_pipeline_closures) / sizeof(uel_closure_t));

#endif /* end of include guard: UEL_PIPELINE_H */
