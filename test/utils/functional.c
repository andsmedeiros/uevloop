#include "functional.h"

#include <stdlib.h>

#include "../uelt.h"
#include "utils/functional.h"
#include "math.h"

#define F(X) (uintptr_t)uel_closure_invoke(&f, (void *)X)

static void *exponentiate(uel_closure_t *closure){
    uintptr_t power = (uintptr_t)closure->context;
    uintptr_t base = (uintptr_t)closure->params;

    return (void *)(uintptr_t)pow(base, power);
}
static void *add(uel_closure_t *closure){
    uintptr_t term1 = (uintptr_t)closure->context;
    uintptr_t term2 = (uintptr_t)closure->params;

    return (void *)(uintptr_t)(term1 + term2);
}

static char *should_operate_pipeline_closure(){
    uel_closure_t square = uel_closure_create(exponentiate, (void *)2, NULL);
    uel_closure_t increment = uel_closure_create(add, (void *)1, NULL);
    UEL_PIPELINE_DECLARE(math, square, increment);

    uel_closure_t f = uel_func_pipeline(&math_pipeline);

    uelt_assert_pointers_equal("f.context", &math_pipeline, f.context);
    uelt_assert_ints_equal("f(0)", 1, F(0));
    uelt_assert_ints_equal("f(3)", 10, F(3));
    uelt_assert_ints_equal("f(5)", 26, F(5));
    uelt_assert_ints_equal("f(10)", 101, F(10));

    return NULL;
}

static void *is_divisible(uel_closure_t *closure){
    uintptr_t divisor = (uintptr_t)closure->context;
    uintptr_t dividend = (uintptr_t)closure->params;

    return (void *)(uintptr_t)(dividend % divisor == 0);
}

static char *should_operate_conditional_closure(){
    uel_closure_t square = uel_closure_create(exponentiate, (void *)2, NULL);
    uel_closure_t increment = uel_closure_create(add, (void *)1, NULL);
    uel_closure_t is_even = uel_closure_create(is_divisible, (void *)2, NULL);

    uel_conditional_t math_conditional;
    uel_conditional_init(&math_conditional, is_even, square, increment);

    uel_closure_t f = uel_func_conditional(&math_conditional);

    uelt_assert_pointers_equal("f.context", &math_conditional, f.context);
    uelt_assert_ints_equal("f(0)", 0, F(0));
    uelt_assert_ints_equal("f(3)", 4, F(3));
    uelt_assert_ints_equal("f(5)", 6, F(5));
    uelt_assert_ints_equal("f(10)", 100, F(10));

    return NULL;
}

static void *accumulate(uel_closure_t *closure){
    uintptr_t *destination = (uintptr_t *)closure->context;
    uintptr_t value = *(uintptr_t *)closure->params;

    *destination += value;
    return (void *)(uintptr_t)true;
}
static void *deref_context(uel_closure_t *closure){
    return (void *)*(uintptr_t *)closure->context;
}
static char *should_operate_foreach_closure(){
    uintptr_t acc = 0;
    uel_closure_t accumulate_into_acc =
        uel_closure_create(accumulate, (void *)&acc, NULL);
    uel_closure_t accumulate_array = uel_func_foreach(&accumulate_into_acc);
    uelt_assert_pointers_equal(
        "accumulate_array.context",
        &accumulate_into_acc,
        accumulate_array.context
    );

    uel_closure_t yield_acc = uel_closure_create(deref_context, (void *)&acc, NULL);
    UEL_PIPELINE_DECLARE(f, accumulate_array, yield_acc);
    uel_closure_t f = uel_func_pipeline(&f_pipeline);

    uintptr_t v1_nums[] = { 0, 0, 0, 0, 0 };
    uel_iterator_array_t v1 = uel_iterator_array_create(v1_nums, 5, sizeof(uintptr_t));
    uelt_assert_ints_equal("f(v1)", 0, F(&v1));

    acc = 0;
    uintptr_t v2_nums[] = { 0, 1, 2, 3, 4 };
    uel_iterator_array_t v2 = uel_iterator_array_create(v2_nums, 5, sizeof(uintptr_t));
    uelt_assert_ints_equal("f(v2)", 10, F(&v2));

    acc = 0;
    uintptr_t v3_nums[] = { 1, 1, 1, 1, 1 };
    uel_iterator_array_t v3 = uel_iterator_array_create(v3_nums, 5, sizeof(uintptr_t));
    uelt_assert_ints_equal("f(v3)", 5, F(&v3));

    acc = 0;
    uintptr_t v4_nums[] = { 1, 2, 3, 5, 7 };
    uel_iterator_array_t v4 = uel_iterator_array_create(v4_nums, 5, sizeof(uintptr_t));
    uelt_assert_ints_equal("f(v4)", 18, F(&v4));

    return NULL;
}

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define UEL_TEST_MAPPER_CONTEXT(id, ...)                                    \
    uintptr_t id##_nums[] = {__VA_ARGS__};                                  \
    uel_iterator_array_t id##_iterator = uel_iterator_array_create(         \
        id##_nums,                                                          \
        ARRAY_SIZE(id##_nums),                                              \
        sizeof(uintptr_t)                                                   \
    );                                                                      \
    void *id[ARRAY_SIZE(id##_nums)]= {0};                                   \
    uel_func_mapper_t id##_mapper;                                          \
    uel_func_mapper_init(&id##_mapper, (uel_iterator_t *)&id##_iterator,    \
                                            id, ARRAY_SIZE(id##_nums));

static void *deref_uintptr_param(uel_closure_t *closure){
    return (void *)*(uintptr_t *)closure->params;
}
static char *should_operate_map_closure(){
    uel_closure_t squared = uel_closure_create(exponentiate, (void *)2, NULL);
    UEL_PIPELINE_DECLARE(square_array, {deref_uintptr_param}, squared);
    uel_closure_t square_array = uel_func_pipeline(&square_array_pipeline);

    uel_closure_t f = uel_func_map(&square_array);

    UEL_TEST_MAPPER_CONTEXT(v1, 0, 1, 2, 3, 4);
    F(&v1_mapper);
    uelt_assert_ints_equal("v1[0]", 0, (uintptr_t)v1[0]);
    uelt_assert_ints_equal("v1[1]", 1, (uintptr_t)v1[1]);
    uelt_assert_ints_equal("v1[2]", 4, (uintptr_t)v1[2]);
    uelt_assert_ints_equal("v1[3]", 9, (uintptr_t)v1[3]);
    uelt_assert_ints_equal("v1[4]", 16, (uintptr_t)v1[4]);

    UEL_TEST_MAPPER_CONTEXT(v2, 1, 1, 1, 1, 1);
    F(&v2_mapper);
    uelt_assert_ints_equal("v2[0]", 1, (uintptr_t)v2[0]);
    uelt_assert_ints_equal("v2[1]", 1, (uintptr_t)v2[1]);
    uelt_assert_ints_equal("v2[2]", 1, (uintptr_t)v2[2]);
    uelt_assert_ints_equal("v2[3]", 1, (uintptr_t)v2[3]);
    uelt_assert_ints_equal("v2[4]", 1, (uintptr_t)v2[4]);

    UEL_TEST_MAPPER_CONTEXT(v3, 1, 2, 3, 5, 7);
    F(&v3_mapper);
    uelt_assert_ints_equal("v3[0]", 1, (uintptr_t)v3[0]);
    uelt_assert_ints_equal("v3[1]", 4, (uintptr_t)v3[1]);
    uelt_assert_ints_equal("v3[2]", 9, (uintptr_t)v3[2]);
    uelt_assert_ints_equal("v3[3]", 25, (uintptr_t)v3[3]);
    uelt_assert_ints_equal("v3[4]", 49, (uintptr_t)v3[4]);

    return NULL;
}

static void *deref_bool_param(uel_closure_t *closure){
    return (void *)*(bool *)closure->params;
}
static char *should_operate_find_closure(){
    uel_closure_t is_true = uel_closure_create(deref_bool_param, NULL, NULL);
    uel_closure_t f = uel_func_find(&is_true);

    bool v1_values[] = {true, false, false, true, false};
    uel_iterator_array_t v1 = uel_iterator_array_create(v1_values, 5, sizeof(bool));
    uelt_assert_pointers_equal("f(v1)", &v1_values[0], F(&v1));

    bool v2_values[] = {false, false, false, true, false};
    uel_iterator_array_t v2 = uel_iterator_array_create(v2_values, 5, sizeof(bool));
    uelt_assert_pointers_equal("f(v2)", &v2_values[3], F(&v2));

    bool v3_values[] = {false, false, false, false, false};
    uel_iterator_array_t v3 = uel_iterator_array_create(v3_values, 5, sizeof(bool));
    uelt_assert_pointer_null("f(v3)", F(&v3));

    return NULL;
}

static char *should_operate_count_closure(){
    uel_closure_t is_true = uel_closure_create(deref_bool_param, NULL, NULL);
    uel_closure_t f = uel_func_count(&is_true);

    bool v1_values[] = {true, false, false, true, false};
    uel_iterator_array_t v1 = uel_iterator_array_create(v1_values, 5, sizeof(bool));
    uelt_assert_ints_equal("f(v1)", 2, F(&v1));

    bool v2_values[] = {false, false, false, true, false};
    uel_iterator_array_t v2 = uel_iterator_array_create(v2_values, 5, sizeof(bool));
    uelt_assert_ints_equal("f(v2)", 1, F(&v2));

    bool v3_values[] = {false, false, false, false, false};
    uel_iterator_array_t v3 = uel_iterator_array_create(v3_values, 5, sizeof(bool));
    uelt_assert_int_zero("f(v3)", F(&v3));

    bool v4_values[] = {true, true, true, true, true};
    uel_iterator_array_t v4 = uel_iterator_array_create(v4_values, 5, sizeof(bool));
    uelt_assert_ints_equal("f(v4)", 5, F(&v4));

    return NULL;
}

static char *should_operate_all_none_any_closure(){
    uel_closure_t is_true = uel_closure_create(deref_bool_param, NULL, NULL);
    uel_closure_t all_true = uel_func_all(&is_true);
    uel_closure_t all_false = uel_func_none(&is_true);
    uel_closure_t any_true = uel_func_any(&is_true);

    bool v1_values[] = {true, false, false, true, false};
    uel_iterator_array_t v1 = uel_iterator_array_create(v1_values, 5, sizeof(bool));
    uelt_assert_not("v1 is not all trues", uel_closure_invoke(&all_true, &v1));
    uelt_assert_not("v1 is not all falses", uel_closure_invoke(&all_false, &v1));
    uelt_assert("v1 has trues", uel_closure_invoke(&any_true, &v1));

    bool v2_values[] = {false, false, false, false, false};
    uel_iterator_array_t v2 = uel_iterator_array_create(v2_values, 5, sizeof(bool));
    uelt_assert_not("v2 is not all trues", uel_closure_invoke(&all_true, &v2));
    uelt_assert("v2 is all falses", uel_closure_invoke(&all_false, &v2));
    uelt_assert_not("v2 has no trues", uel_closure_invoke(&any_true, &v2));

    bool v3_values[] = {true, true, true, true, true};
    uel_iterator_array_t v3 = uel_iterator_array_create(v3_values, 5, sizeof(bool));
    uelt_assert("v3 is all trues", uel_closure_invoke(&all_true, &v3));
    uelt_assert_not("v3 is not all falses", uel_closure_invoke(&all_false, &v3));
    uelt_assert("v3 has trues", uel_closure_invoke(&any_true, &v3));

    return NULL;
}
char *uel_functional_run_tests(){

    uelt_run_test(
        "should correctly operate pipeline closure",
        should_operate_pipeline_closure
    );
    uelt_run_test(
        "should correctly operate conditional closure",
        should_operate_conditional_closure
    );
    uelt_run_test(
        "should correctly operate foreach closure",
        should_operate_foreach_closure
    );
    uelt_run_test(
        "should correctly operate map closure",
        should_operate_map_closure
    );
    uelt_run_test(
        "should correctly operate find closure",
        should_operate_find_closure
    );
    uelt_run_test(
        "should correctly operate count closure",
        should_operate_count_closure
    );
    uelt_run_test(
        "should correctly operate all/none/any closures",
        should_operate_all_none_any_closure
    );

    return NULL;
}

#undef F
#undef ARRAY_SIZE
