/** \file conditional.h
  *
  * \brief Contains definitions of functional conditionals, structures that act
  * as if-else constructs.
  */

#ifndef UEL_CONDITIONAL_H
#define UEL_CONDITIONAL_H

#include "uevloop/utils/closure.h"

/** \brief Conditionals are constructs that provide functional flow control.
  *
  * A conditional is defined by a tuple <test, if_true, if_false> of closures.
  * When applied to some input, this input is passed to the `test` closure.
  * Based on its returned value, one of the other closures is invoked.
  */
typedef struct uel_conditional uel_conditional_t;
struct uel_conditional{
    //! A closure that, based on its input, should return `true` or `false`.
    uel_closure_t test;
    //! The closure that will be invoked if the `test` closure returns `true`
    uel_closure_t if_true;
    //! The closure that will be invoked if the `test` closure returns `false`
    uel_closure_t if_false;
};

/** \brief Initialises a conditional structure
  *
  * \param conditional The conditional object to be initialised
  * \param test The closure to test the input in order to decide what to do next
  * \param if_true The closure to be invoked when the test closure returns `true`
  * \param if_false The closure to be invoked when the test closure returns `false`
  */
void uel_conditional_init(uel_conditional_t *conditional, uel_closure_t test,
                                uel_closure_t if_true, uel_closure_t if_false);

/** \brief Applies a conditional do some input
  *
  * This function takes some input and submits it to the test closure, whose
  * output will be cast to a boolean value.
  * According to this value, either `if_true` or `if_false` closures will be called
  * with the *same value* as parameters.
  *
  * \param conditional The conditional to be applied
  * \param params The parameter that will be tested and provided to the chosen
  * closure
  * \returns Whatever the invoked closure returned
  */
void *uel_conditional_apply(uel_conditional_t *conditional, void *params);

#endif /* end of include guard: UEL_CONDITIONAL_H */
