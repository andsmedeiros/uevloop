/** \file automatic-pool.h
  *
  * \brief Defines automatic pointers and automatic pools, objects that wrap
  * object pools and objects managed by them. These wrappers provide basic
  * automatic memory management.
  */

#ifndef UEL_AUTOMATIC_POOL_H
#define UEL_AUTOMATIC_POOL_H

#include "uevloop/utils/object-pool.h"
#include "uevloop/utils/closure.h"

struct uel_autopool;

/** \brief Automatic pointers are pointers wrapped by information of where they
  * were issued.
  *
  * Upon deallocation, an object wrapped by an autoptr is automatically
  * returned to its original pool.
  * Automatic pointers are meant to be pooled themselves.
  *
  * The programmer can safely cast a `uel_autoptr_t` to a `void *` or a pointer
  * to whatever type the wrapped object is.
  */
struct uel_autoptr {
    void *object; //!< A pointer to the wrapped object
    struct uel_autopool *source; //!< The pool where the autoptr object was issued
};

/** \brief Aliases `uel_autoptr_t` to `void **` so it can be cast to pointers to
  * other types
  */
typedef void ** uel_autoptr_t;

/** \brief Deallocates an automatic pointer.
  *
  * When called, the destructor closure defined at the autopointer's source
  * autopool is invoked. Afterwards, the autopointer is returned to its pool.
  *
  * \see uel_autopool_set_destructor()
  * \warning The object **must** not be used after it's dealloc'ed.
  */
void uel_autoptr_dealloc(uel_autoptr_t autoptr);

/** \brief Automatic pools are wrappers to object pools. They manage the
  * acquisition and release cycle from objects issued at the pool by wrapping
  * them in automatic pointers.
  */
typedef struct uel_autopool uel_autopool_t;
struct uel_autopool {
    uel_objpool_t autoptr_pool; //!< The object pool that holds autopointers
    uel_closure_t constructor; //!< The constructor closure
    uel_closure_t destructor; //!< The destructor closure
};

/** \brief Initialises an automatic pool
  *
  * \param pool The pool to be initialised
  * \param size_log2n The number of objects in the pool in its log2 form
  * \param item_size The size of each object in the pool. If special alignment
  * is required, it must be included in this value.
  * \param object_buffer The buffer that contains each object in the pool. Must
  * be `2**size_log2n * item_size` long.
  * \param autoptr_buffer The buffer that contains each autoptr object to be issued.
  * Must be `2**size_log2n * item_size` long.
  * \param queue_buffer A void pointer array that will be used as the buffer to
  * the object pointer queue. Must be `2**size_log2n` long.
  */
void uel_autopool_init(
    uel_autopool_t *pool,
    size_t size_log2n,
    size_t item_size,
    uint8_t *object_buffer,
    struct uel_autoptr *autoptr_buffer,
    void **queue_buffer
);

/** \brief Allocates an object and wrap it in a automatic pointer.
  *
  * The allocated object is submited to the contructor closure set in the
  * autopool.
  *
  * \see uel_autopool_set_constructor()
  *
  * \param pool The automatic pool from where to acquire the object
  * \returns An autopointer wrapping the acquired object or NULL if one could
  * not be acquired.
  */
uel_autoptr_t uel_autopool_alloc(uel_autopool_t *pool);


/** \brief Checks if a pool is depleted
  *
  * \param pool The pool to be verified
  * \return Whether the pool is empty (*i.e.*: All autoptrs have been given out)
  */
bool uel_autopool_is_empty(uel_autopool_t *pool);

/** \brief Sets the constructor closure of an autopool.
  *
  * This closure is invoked when `uel_autoptr_alloc()` is called and takes a
  * bare pointer to the object being alloc'ed as parameter.
  *
  * \see uel_autoptr_alloc()
  *
  * \param pool The autopool onto which to attach the constructor
  * \param constructor The constructor closure
  */
void uel_autopool_set_constructor(uel_autopool_t *pool, uel_closure_t constructor);

/** \brief Sets the destructor closure of an autopool.
  *
  * This closure is invoked when `uel_autoptr_dealloc()` is called and takes a
  * bare pointer to the object being dealloc'ed as parameter.
  *
  * \see uel_autoptr_dealloc()
  *
  * \param pool The autopool onto which to attach the destructor
  * \param destructor The destructor closure
  */
void uel_autopool_set_destructor(uel_autopool_t *pool, uel_closure_t destructor);

/** \brief Declares the necessary buffers to back an automatic pool, so the
  * programmer doesn't have to reason much about it.
  *
  * Use this macro as a shortcut to create the required buffers for an automatic
  * pool. This will declare three buffers in the calling scope.
  *
  * \param type The type of the objects the pool will contain
  * \param size_log2n The number of elements the pool will contain in log2 form
  * \param id A valid identifier for the pools.
  */
#define UEL_DECLARE_AUTOPOOL_BUFFERS(type, size_log2n, id)          \
    type id##_buffer[(1<<size_log2n)];                              \
    struct uel_autoptr id##_pool_buffer[1<<size_log2n];             \
    void *id##_pool_queue_buffer[1<<size_log2n];

/** \brief Refers to a previously declared buffer set.
  *
  * This is a convenience macro to supply the buffers generated by
  * `UEL_DECLARE_AUTOPOOL_BUFFERS()` to the `uel_autopool_init()` function.
  *
  * \param id The identifier used to declare the pool buffers
  */
#define UEL_AUTOPOOL_BUFFERS(id)                                     \
    (uint8_t *)&id##_buffer, id##_pool_buffer, id##_pool_queue_buffer

/** \brief Refers to a previously declared buffer set.
  *
  * This is a convenience macro to supply the buffers generated by
  * `UEL_DECLARE_AUTOPOOL_BUFFERS()` to the `uel_autopool_init()` function.
  * Use this if the buffers were defined inside a local object, accessible in the
  * current scope.
  *
  * \param id The identifier used to declare the pool buffers
  * \param obj The object storing the pool buffers
  */
#define UEL_AUTOPOOL_BUFFERS_IN(id, obj)                            \
    (uint8_t *)&obj.id##_buffer, obj.id##_pool_buffer,              \
    obj.id##_pool_queue_buffer

/** \brief Refers to a previously declared buffer set.
  *
  * This is a convenience macro to supply the buffers generated by
  * `UEL_DECLARE_AUTOPOOL_BUFFERS()` to the `uel_autopool_init()` function.
  * Use this if the buffers were defined inside an object whose address is
  * accessible in the current scope
  *
  * \param id The identifier used to declare the pool buffers
  * \param obj The address of the object storing the pool buffers
  */
#define UEL_AUTOPOOL_BUFFERS_AT(id, obj)                            \
    (uint8_t *)&obj->id##_buffer, obj->id##_pool_buffer,            \
    obj->id##_pool_queue_buffer

#endif /* end of include guard: UEL_AUTOMATIC_POOL_H */
