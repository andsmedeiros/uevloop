#include <stdint.h>
#include <stdbool.h>

#include "../uelt.h"
#include "utils/conditional.h"

static void *is_divisible(uel_closure_t *closure){
    uintptr_t divisor = (uintptr_t)closure->context;
    uintptr_t dividend = (uintptr_t)closure->params;
    return (void *)(uintptr_t)((dividend % divisor) == 0);
}

static void *store_number(uel_closure_t *closure){
    uintptr_t *target =  (uintptr_t *)closure->context;
    uintptr_t number = (uintptr_t)closure->params;
    *target = number;
    return NULL;
}

char *should_initialise_conditional(){
    uintptr_t num1 = 0, num2 = 0;
    uel_closure_t is_even = uel_closure_create(is_divisible, (void *)2, NULL);
    uel_closure_t store_at_num1 = uel_closure_create(store_number, &num1, NULL);
    uel_closure_t store_at_num2 = uel_closure_create(store_number, &num2, NULL);

    uel_conditional_t conditional;
    uel_conditional_init(&conditional, is_even, store_at_num1, store_at_num2);

    uelt_assert_pointers_equal(
        "conditional.test.function",
        &is_divisible,
        conditional.test.function
    );
    uelt_assert_ints_equal("conditional.test.context", 2, (uintptr_t)conditional.test.context);
    uelt_assert_pointers_equal(
        "conditional.if_true.function",
        &store_number,
        conditional.if_true.function
    );
    uelt_assert_pointers_equal(
        "conditional.if_true.context",
        &num1,
        conditional.if_true.context
    );
    uelt_assert_pointers_equal(
        "conditional.if_false.function",
        &store_number,
        conditional.if_false.function
    );
    uelt_assert_pointers_equal(
        "conditional.if_false.context",
        &num2,
        conditional.if_false.context
    );

    return NULL;
}

char *should_apply_conditional(){
    uintptr_t num1 = 0, num2 = 0;
    uel_closure_t is_even = uel_closure_create(is_divisible, (void *)2, NULL);
    uel_closure_t store_at_num1 = uel_closure_create(store_number, &num1, NULL);
    uel_closure_t store_at_num2 = uel_closure_create(store_number, &num2, NULL);

    uel_conditional_t conditional;
    uel_conditional_init(&conditional, is_even, store_at_num1, store_at_num2);

    uel_conditional_apply(&conditional, (void *)3);
    uelt_assert_int_zero("applying 3, num1", num1);
    uelt_assert_ints_equal("applying 3, num2", 3, num2);

    uel_conditional_apply(&conditional, (void *)4);
    uelt_assert_ints_equal("applying 4, num1", 4, num1);
    uelt_assert_ints_equal("applying 4, num2", 3, num2);

    uel_conditional_apply(&conditional, (void *)7);
    uelt_assert_ints_equal("applying 7, num1", 4, num1);
    uelt_assert_ints_equal("applying 7, num2", 7, num2);

    return NULL;
}

char *uel_conditional_run_tests(){
    uelt_run_test("should correctly initialise a conditional object", should_initialise_conditional);
    uelt_run_test("should correctly apply a conditional object", should_apply_conditional);

    return NULL;
}
