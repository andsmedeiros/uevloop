/** \file promise.h
  * \brief Contains definitions for promise stores, promises and functions to
  * manipulate them.
  *
  * Promises are structures that associate asynchronous operations to synchronous
  * processing of the values produces by them, allowing async data pipelines to
  * be built.
  */

#ifndef UEL_PROMISE_H
#define UEL_PROMISE_H

#include "uevloop/config.h"
#include "uevloop/utils/closure.h"
#include "uevloop/utils/object-pool.h"

/** \brief Defines the possible states for a prommise
  */
enum uel_promise_state {
    UEL_PROMISE_PENDING , //!< A promise that has not been resolved nor rejected
    UEL_PROMISE_RESOLVED, //!< A promise that has been resolved with some value
    UEL_PROMISE_REJECTED, //!< A promise that has been rejected with some error
};
//! Alias to the `uel_promise_state` enum
typedef enum uel_promise_state uel_promise_state_t;

/** \brief Defines a single synchronous operation to be invoked when the promise
  * is either resolved or rejected.
  *
  * A promise segment is a data structure that defines how to process a value
  * yielded by a promise when it resolves or rejects.
  *
  * Segments contain two closures, representing success and error execution paths.
  * Once the promise containing a segment is settled, depending on its state,
  * either closure is invoked with the promise as parameter.
  *
  * If a handler closure returns anything different that NULL, it's assumed to
  * be a promise pointer to be awaited for.
  */
typedef struct uel_promise_segment uel_promise_segment_t;
struct uel_promise_segment {
    //! The closure to be invoked when the promise is resolved
    uel_closure_t resolve;
    //! The closure to be invoked when the promise is rejected
    uel_closure_t reject;
    //! The next synchronous segment to be processed
    uel_promise_segment_t *next;
};

//! Forward declaration of the store struct
struct uel_promise_store;

/** \brief A promise is association of an asynchronous operation to the possible
  * execution paths that follow its resolution. It is also a holder for the value
  * it was settled with.
  *
  * A promise is an object that can be in three different states:
  * - PENDING: <br/>
  *     A promise is pending when its asynchronous operation has not completed
  *     yet and the promise does **NOT** hold any meaningful value. <br/>
  *     If a segment handler returns a new promise, the original promise is
  *     considered pending while it awaits for the new promise to settle.
  *
  * - RESOLVED: <br/>
  *     A promise is resolved when its asynchronous operation is successfully
  *     completed. It consistently holds whatever value it has been resolved with.
  *
  * - REJECTED: <br/>
  *     A promise is rejected if its asynchronous operation could not complete
  *     successfully. Its value is set to whatever error it has been resolved with.
  */
typedef struct uel_promise uel_promise_t;
struct uel_promise {
    //! The promise store from where this promise was issued
    struct uel_promise_store *source;
    //! The current state of this promise
    uel_promise_state_t state;
    //! The value this promise holds. Only meaningful if already settled.
    void *value;
    //! The first segment to be processed when this promise settles
    uel_promise_segment_t *first_segment;
    //! The last segment to be processed when this promise settles
    uel_promise_segment_t *last_segment;
};

/** \brief An issuer of promises. Contains references to pools for promises and
  * segments.
  */
typedef struct uel_promise_store uel_promise_store_t;
struct uel_promise_store {
    //! A reference to the promise pool
    uel_objpool_t *promise_pool;
    //! A reference to the segment pool
    uel_objpool_t *segment_pool;
};

/** \brief Creates a new promise store from the promise and segment pools
  *
  * \param promise_pool The `uel_objpool_t` that holds promises
  * \param segment_pool The `uel_objpool_t` that holds segments
  * \return A new promise store bound to the object pools provided
  */
uel_promise_store_t uel_promise_store_create(
    uel_objpool_t *promise_pool,
    uel_objpool_t *segment_pool
);

/** \brief Acquires a new promise from the supplied store and binds it to the
  * asynchronous operation started by the supplied closure. The closure is
  * invoked immediately.
  *
  * \param store The store from where to acquire promises and segments
  * \param closure The closure that initiates the asynchronous operation
  * \returns A pointer to the promise
  */
uel_promise_t *uel_promise_create(uel_promise_store_t *store, uel_closure_t closure);

/** \brief Destroys a promise and all of its segments. Settling this promise
  * afterwards yields undefined behaviour.
  *
  * \param promise The promise to be destroyed
  */
void uel_promise_destroy(uel_promise_t *promise);

/** \brief Adds a new synchronous segment to the promise. It will be invoked
  * upon promise resolution. In case of rejection, this segment will be ignored.
  *
  * \param promise The promise to attach the segment to
  * \param resolve The closure to  be invoked when the promise is resolved
  */
void uel_promise_then(uel_promise_t *promise, uel_closure_t resolve);

/** \brief Adds a new synchronous segment to the promise. It will be invoked
  * upon promise rejection. In case of resolution, this segment will be ignored.
  *
  * \param promise The promise to attach the segment to
  * \param reject The closure to  be invoked when the promise is rejected
  */
void uel_promise_catch(uel_promise_t *promise, uel_closure_t reject);

/** \brief Adds a new synchronous segment to the promise. The same closure will
  * be invoked on promise settling regardless of the settled state.
  *
  * \param promise The promise to attach the segment to
  * \param always The closure to be invoked when the promise is settled
  */
void uel_promise_always(uel_promise_t *promise, uel_closure_t always);

/** \brief Adds a new synchronous segment to the promise. Either of its closures
  * will be invoked, depending on the settled state of the promise.
  *
  * \param promise The promise to attach the segment to
  * \param resolve The closure to be invoked when the promise is resolved
  * \param reject The closure to be invoked when the promise is rejected
  */
void uel_promise_after(
    uel_promise_t *promise,
    uel_closure_t resolve,
    uel_closure_t reject
);

/** \brief Settles a promise as resolved and, **synchronously**, invokes the
  * `resolve` closures of each segment in the order they were registered.
  *
  * If a segment returns a non-NULL pointer, it is cast to a promise pointer
  * and the original promise awaits until the returned promise is settled.
  *
  * \param promise The promise to be resolved
  * \param value The value to resolve the promise with
  */
void uel_promise_resolve(uel_promise_t *promise, void *value);

/** \brief Settles a promise as rejected and, **synchronously**, invokes the
  * `reject` closures of each segment in the order they were registered.
  *
  * If a segment returns a non-NULL pointer, it is cast to a promise pointer
  * and the original promise awaits until the returned promise is settled.
  *
  * \param promise The promise to be rejected
  * \param value The value to reject the promise with
  */
void uel_promise_reject(uel_promise_t *promise, void *value);

/** \brief Settles a promise as the supplied state. Unlike `uel_promise_resolve()`
  * and `uel_promise_reject()`, does **not** invoke the synchronous segments.
  *
  * This function should be used to switch execution paths during synchronous
  * processing of segments as to signal that an error was raised or rescued
  * by a particular segment. If the promise is settled as pending, the synchronous
  * processing phase is interrupted.
  *
  * \param promise The promise to be settled
  * \param state The new promise state
  * \param value The new promise value
  */
void uel_promise_settle(
    uel_promise_t *promise,
    uel_promise_state_t state,
    void *value
);

/** \brief Creates a closure bound to a promise. When the closure is invoked
  * with some parameter, the promise is resolved with this parameter as value.
  *
  * \param promise The promise to be resolved
  * \returns A closure that resolves the promise when invoked
  */
uel_closure_t uel_promise_resolver(uel_promise_t *promise);

/** \brief Creates a closure bound to a promise. When the closure is invoked
  * with some parameter, the promise is rejected with this parameter as error.
  *
  * \param promise The promise to be rejected
  * \returns A closure that resolves the promise when invoked
  */
uel_closure_t uel_promise_rejecter(uel_promise_t *promise);

/** \brief Creates a closure bound to a promise. When the closure is invoked,
  * the promise is destroyed. Any parameters passed to the closure are ignored.
  *
  * \param promise The promise to be destroyed
  * \returns A closure that destroyes the promise when invoked
  */
uel_closure_t uel_promise_destroyer(uel_promise_t *promise);

#ifdef UEL_PROMISE_SHORTCUTS

#define THEN    uel_promise_then
#define CATCH   uel_promise_catch
#define AFTER   uel_promise_after
#define ALWAYS  uel_promise_always


#endif /* UEL_PROMISE_SHORTCUTS */

#endif /* end of include guard: UEL_PROMISE_H */
