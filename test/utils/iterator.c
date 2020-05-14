#include "iterator.h"

#include <stdlib.h>

#include "../uelt.h"
#include "uevloop/utils/iterator.h"

#define UEL_TEST_DECLARE_ITERATOR_ARRAY()   \
    int nums[] = {1, 2, 3, 4, 5};           \
    uel_iterator_array_t iterator = uel_iterator_array_create(nums, 5, sizeof(int));

static char *should_create_array_iterator(){
    UEL_TEST_DECLARE_ITERATOR_ARRAY();

    uelt_assert_pointers_equal(
        "iterator.base.collection",
        &nums,
        iterator.base.collection
    );
    uelt_assert_pointers_equal(
        "iterator.base.next",
        &uel_iterator_array_next,
        iterator.base.next
    );
    uelt_assert_ints_equal("iterator.item_count", 5, iterator.item_count);
    uelt_assert_ints_equal("iterator.item_size", sizeof(int), iterator.item_size);

    return NULL;
}

static char *should_iterate_array(){
    UEL_TEST_DECLARE_ITERATOR_ARRAY();

    void *current = NULL;
    for(size_t i = 0; i < 5; i++){
        current = uel_iterator_array_next(&iterator, current);
        uelt_assert_pointers_equal(
            "yielded pointer on array iteration",
            &nums[i],
            (int *)current
        );
    }
    current = uel_iterator_array_next(&iterator, current);
    uelt_assert_pointer_null("yielded pointer after last element", current);

    return NULL;
}

#define UEL_TEST_DECLARE_ITERATOR_LLIST()           \
    uel_llist_node_t nodes[] = {                    \
        {(void *)1, &nodes[1]},                     \
        {(void *)2, &nodes[2]},                     \
        {(void *)3, &nodes[3]},                     \
        {(void *)4, &nodes[4]},                     \
        {(void *)5, NULL}                           \
    };                                              \
    uel_llist_t list = { &nodes[5], &nodes[0] };    \
    uel_iterator_llist_t iterator = uel_iterator_llist_create(&list);

static char *should_create_llist_iterator(){
    UEL_TEST_DECLARE_ITERATOR_LLIST();

    uelt_assert_pointers_equal("iterator.collection", &list, iterator.collection);
    uelt_assert_pointers_equal("iterator.next", &uel_iterator_llist_next, iterator.next);

    return NULL;
}

static char *should_iterate_llist(){
    UEL_TEST_DECLARE_ITERATOR_LLIST();

    void *current = NULL;
    for(size_t i = 0; i < 5; i++){
        current = uel_iterator_llist_next(&iterator, current);
        uelt_assert_pointers_equal(
            "yielded pointer on linked list iteration",
            &nodes[i],
            (int *)current
        );
    }
    current = uel_iterator_llist_next(&iterator, current);
    uelt_assert_pointer_null("yielded pointer after last element", current);

    return NULL;
}

static void *accumulate(uel_closure_t *closure){
    int *acc = (int *)closure->context;
    int number = *(int *)closure->params;

    *acc += number;
    return (void *)true;
}

static void *count_until_multiple_of_five(uel_closure_t *closure){
    int number = *(int *)closure->params;
    if(number % 5 != 0){
        int *counter = (int *)closure->context;
        (*counter)++;
        return (void *)true;
    }else{
        return (void *)false;
    }
}
static char *should_operate_iterator_foreach(){
    UEL_TEST_DECLARE_ITERATOR_ARRAY();

    int sum = 0;
    uel_closure_t closure1 = uel_closure_create(accumulate, (void *)&sum, NULL);

    bool success = uel_iterator_foreach((uel_iterator_t *)&iterator, &closure1);
    uelt_assert_ints_equal("sum of array members", 15, sum);
    uelt_assert("iterator finished successfully", success);

    int counter = 0;
    uel_closure_t closure2 =
        uel_closure_create(count_until_multiple_of_five, (void *)&counter, NULL);

    success = uel_iterator_foreach((uel_iterator_t *)&iterator, &closure2);
    uelt_assert_ints_equal(
        "count of elements until a multiple of five is found",
        4,
        counter
    );
    uelt_assert_not("iterator finished successfully", success);

    return NULL;
}

static void *increment(uel_closure_t *closure){
    int number = *(int *)closure->params;
    return (void *)(uintptr_t)(number + 1);
}

static char *should_operate_iterator_map(){
    UEL_TEST_DECLARE_ITERATOR_LLIST();

    void *incremented[7] = {0};
    uel_closure_t closure = uel_closure_create(increment, NULL, NULL);

    size_t iterated =
        uel_iterator_map((uel_iterator_t *)&iterator, &closure, incremented, 5);
    for (size_t i = 0; i < 5; i++) {
        uelt_assert_ints_equal(
            "incremented mapped numbers with limit=5",
            i + 2,
            (uintptr_t)incremented[i]
        );
        incremented[i] = NULL;
    }
    uelt_assert_ints_equal("number of mapped elements with limit=5", 5, iterated);

    iterated =
        uel_iterator_map((uel_iterator_t *)&iterator, &closure, incremented, 3);
    for (size_t i = 0; i < 3; i++) {
        uelt_assert_ints_equal(
            "incremented mapped numbers with limit=3",
            i + 2,
            (uintptr_t)incremented[i]
        );
        incremented[i] = NULL;
    }
    uelt_assert_int_zero(
        "number at incremented[3] with limit=3",
        (uintptr_t)incremented[3]
    );
    uelt_assert_int_zero(
        "number at incremented[4] with limit=3",
        (uintptr_t)incremented[4]
    );
    uelt_assert_ints_equal("number of mapped elements with limit=3", 3, iterated);

    iterated =
        uel_iterator_map((uel_iterator_t *)&iterator, &closure, incremented, 7);
    for (size_t i = 0; i < 5; i++) {
        uelt_assert_ints_equal(
            "incremented mapped numbers with limit=7",
            i + 2,
            (uintptr_t)incremented[i]
        );
        incremented[i] = NULL;
    }
    uelt_assert_int_zero(
        "number at incremented[5] with limit=7",
        (uintptr_t)incremented[5]
    );
    uelt_assert_int_zero(
        "number at incremented[6] with limit=7",
        (uintptr_t)incremented[6]
    );
    uelt_assert_ints_equal("number of mapped elements with limit=7", 5, iterated);

    return NULL;
}

static void *is_divisible(uel_closure_t *closure){
    uintptr_t divisor = (uintptr_t)closure->context;
    int dividend = *(int *)closure->params;

    return (void *)(uintptr_t)((dividend % divisor) == 0);
}
static char *should_operate_iterator_find(){
    UEL_TEST_DECLARE_ITERATOR_ARRAY()

    uel_closure_t is_divisible_by_1 = uel_closure_create(is_divisible, (void *)1, NULL);
    void *div_by_1 = uel_iterator_find((uel_iterator_t *)&iterator, &is_divisible_by_1);
    uelt_assert_pointers_equal("div_by_1", &nums[0], div_by_1);

    uel_closure_t is_divisible_by_2 = uel_closure_create(is_divisible, (void *)2, NULL);
    void *div_by_2 = uel_iterator_find((uel_iterator_t *)&iterator, &is_divisible_by_2);
    uelt_assert_pointers_equal("div_by_2", &nums[1], div_by_2);

    uel_closure_t is_divisible_by_5 = uel_closure_create(is_divisible, (void *)5, NULL);
    void *div_by_5 = uel_iterator_find((uel_iterator_t *)&iterator, &is_divisible_by_5);
    uelt_assert_pointers_equal("div_by_5", &nums[4], div_by_5);

    uel_closure_t is_divisible_by_10 = uel_closure_create(is_divisible, (void *)10, NULL);
    void *div_by_10 = uel_iterator_find((uel_iterator_t *)&iterator, &is_divisible_by_10);
    uelt_assert_pointer_null("div_by_10", div_by_10);

    return NULL;
}

static char *should_operate_iterator_count(){
    int nums[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uel_iterator_array_t iterator = uel_iterator_array_create(nums, 10, sizeof(int));

    uel_closure_t is_divisible_by_1 = uel_closure_create(is_divisible, (void *)1, NULL);
    size_t count_div_by_1 = uel_iterator_count((uel_iterator_t *)&iterator, &is_divisible_by_1);
    uelt_assert_ints_equal("count_div_by_1", 10, count_div_by_1);

    uel_closure_t is_divisible_by_2 = uel_closure_create(is_divisible, (void *)2, NULL);
    size_t count_div_by_2 = uel_iterator_count((uel_iterator_t *)&iterator, &is_divisible_by_2);
    uelt_assert_ints_equal("count_div_by_2", 5, count_div_by_2);

    uel_closure_t is_divisible_by_3 = uel_closure_create(is_divisible, (void *)3, NULL);
    size_t count_div_by_3 = uel_iterator_count((uel_iterator_t *)&iterator, &is_divisible_by_3);
    uelt_assert_ints_equal("count_div_by_3", 3, count_div_by_3);

    uel_closure_t is_divisible_by_5 = uel_closure_create(is_divisible, (void *)5, NULL);
    size_t count_div_by_5 = uel_iterator_count((uel_iterator_t *)&iterator, &is_divisible_by_5);
    uelt_assert_ints_equal("count_div_by_5", 2, count_div_by_5);

    uel_closure_t is_divisible_by_10 = uel_closure_create(is_divisible, (void *)10, NULL);
    size_t count_div_by_10 = uel_iterator_count((uel_iterator_t *)&iterator, &is_divisible_by_10);
    uelt_assert_ints_equal("count_div_by_10", 1, count_div_by_10);

    return NULL;
}

void *is_num(uel_closure_t *closure){
    uintptr_t num1 = (uintptr_t)closure->context;
    int *num2 = (int *)closure->params;
    return (void *)(uintptr_t)(*num2 == num1);
}
static char *should_operate_iterator_all_none_any(){
    uel_closure_t is_one = uel_closure_create(is_num, (void *)1, NULL);
    uel_closure_t is_zero = uel_closure_create(is_num, (void *)0, NULL);

    int all_ones[] = { 1, 1, 1, 1, 1 };
    uel_iterator_array_t ones_iterator = uel_iterator_array_create(all_ones, 5, sizeof(int));
    uelt_assert(
        "all_ones only have ones",
        uel_iterator_all((uel_iterator_t *)&ones_iterator, &is_one)
    );
    uelt_assert_not(
        "all_ones have no zeros",
        uel_iterator_all((uel_iterator_t *)&ones_iterator, &is_zero)
    );
    uelt_assert(
        "all_ones have no zeros",
        uel_iterator_none((uel_iterator_t *)&ones_iterator, &is_zero)
    );
    uelt_assert_not(
        "all_ones does have ones",
        uel_iterator_none((uel_iterator_t *)&ones_iterator, &is_one)
    );
    uelt_assert(
        "all_ones have some ones",
        uel_iterator_any((uel_iterator_t *)&ones_iterator, &is_one)
    );
    uelt_assert_not(
        "all_ones doesn't have any zeros",
        uel_iterator_any((uel_iterator_t *)&ones_iterator, &is_zero)
    );

    int all_zeros[] = { 0, 0, 0, 0, 0 };
    uel_iterator_array_t zeros_iterator = uel_iterator_array_create(all_zeros, 5, sizeof(int));
    uelt_assert(
        "all_zeros only have zeros",
        uel_iterator_all((uel_iterator_t *)&zeros_iterator, &is_zero)
    );
    uelt_assert_not(
        "all_zeros have no ones",
        uel_iterator_all((uel_iterator_t *)&zeros_iterator, &is_one)
    );
    uelt_assert(
        "all_zeros have no ones",
        uel_iterator_none((uel_iterator_t *)&zeros_iterator, &is_one)
    );
    uelt_assert_not(
        "all_zeros does have zeros",
        uel_iterator_none((uel_iterator_t *)&zeros_iterator, &is_zero)
    );
    uelt_assert(
        "all_zeros have some zeros",
        uel_iterator_any((uel_iterator_t *)&zeros_iterator, &is_zero)
    );
    uelt_assert_not(
        "all_zeros doesn't have any ones",
        uel_iterator_any((uel_iterator_t *)&zeros_iterator, &is_one)
    );

    int mixed[] = { 1, 0, 1, 0, 1 };
    uel_iterator_array_t mixed_iterator = uel_iterator_array_create(mixed, 5, sizeof(int));
    uelt_assert_not(
        "mixed is not all zeros",
        uel_iterator_all((uel_iterator_t *)&mixed_iterator, &is_zero)
    );
    uelt_assert_not(
        "mixed is not all ones",
        uel_iterator_all((uel_iterator_t *)&mixed_iterator, &is_one)
    );
    uelt_assert_not(
        "mixed does have ones",
        uel_iterator_none((uel_iterator_t *)&mixed_iterator, &is_one)
    );
    uelt_assert_not(
        "mixed does have zeros",
        uel_iterator_none((uel_iterator_t *)&mixed_iterator, &is_zero)
    );
    uelt_assert(
        "mixed have some zeros",
        uel_iterator_any((uel_iterator_t *)&mixed_iterator, &is_zero)
    );
    uelt_assert(
        "mixed have some ones",
        uel_iterator_any((uel_iterator_t *)&mixed_iterator, &is_one)
    );

    return NULL;
}

void *uel_iterator_run_tests(){

    uelt_run_test(
        "should correctly initialise an array iterator",
        should_create_array_iterator
    );
    uelt_run_test("should correctly iterate over an array", should_iterate_array);
    uelt_run_test(
        "should correctly initialise a linked list iterator",
        should_create_llist_iterator
    );
    uelt_run_test("should correctly iterate over a linked list", should_iterate_llist);
    uelt_run_test(
        "should correctly operate iterators: foreach",
        should_operate_iterator_foreach
    );
    uelt_run_test(
        "should correctly operate iterators: map",
        should_operate_iterator_map
    );
    uelt_run_test(
        "should correctly operate iterators: find",
        should_operate_iterator_find
    );
    uelt_run_test(
        "should correctly operate iterators: count",
        should_operate_iterator_count
    );
    uelt_run_test(
        "should correctly operate iterators: all / none / any",
        should_operate_iterator_all_none_any
    );

    return NULL;
}
