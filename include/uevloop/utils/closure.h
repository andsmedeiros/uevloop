/** \file closure.h
  *
  * \brief Defines closures, objects that bind functions to creating and calling
  * contexts
  */

#ifndef UEL_CLOSURE_H
#define	UEL_CLOSURE_H

/** \brief Defines a closure function, suitable for being bound at a closure.
  *
  * Must take two pointers ar arguments, one for the context and one for
  * closure parameters. Must return a void pointer.
  */
typedef void * (* uel_closure_function_t)(void *context, void *params);

/** \brief Defines a closure, a tuple <function, context, destructor>
  *
  * A closure is a data structure that binds some function to a predetermined
  * context. When the function is run, this context will be available to
  * the function body.
  *
  * When invoking the closure, additional parameters may be passed along and will
  * also be available to the function body. The function can also return a value
  * that will be passed back by the `uel_closure_invoke` function.
  */
typedef struct uel_closure uel_closure_t;
struct uel_closure{
    //! \brief The function to be run when the closure is invoked.
    //! Must take two pointers ar arguments, one for the context and one for
    //! closure parameters. Must return a void pointer.
    uel_closure_function_t function;
    //! \brief The context supplied to the closure durint creation.
    //! This context will be available during the function execution.
    void *context;
};

/** \brief Creates a new closure.
  *
  * Binds the tuple <function, context> and populates a new closure
  * object with it. The object is returned by value.
  * \param function The function to be run on closure invokation.
  * \param context The creation context of the closure.
  * \return The closure object, by value.
  */
uel_closure_t uel_closure_create(uel_closure_function_t function,void *context);

/** \brief Invokes a closure and returns whatever value it returned.
  *
  * \param closure The closure reference to be invoked.
  * \param params The parameters to be passed along during closure invokation.
  * \return This function returns whatever the closure function returned.
  */
void *uel_closure_invoke(uel_closure_t *closure, void *params);

/** \brief Returns a closure that does nothing.
  *
  * The returned closure can be passed as parameters to functions that take
  * closures as callbacks when the caller does not need to callback.
  */
uel_closure_t uel_nop();

#endif	/* UEL_CLOSURE_H */
