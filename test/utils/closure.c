#include "closure.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../uelt.h"
#include "utils/closure.h"

static void *nop1(uel_closure_t *closure){ return NULL; }
static void nop2(uel_closure_t *closure){}
static char *should_create_closure(){
    uint8_t context = 18;

    uel_closure_t closure = uel_closure_create(&nop1, &context, &nop2);
    uelt_assert_pointers_equal("closure.function", closure.function, &nop1);
    uelt_assert_pointers_equal("closure.context", closure.context, &context);
    uelt_assert_pointers_equal("closure.destructor", closure.destructor, &nop2);
    uelt_assert_pointer_null("closure.params", closure.params);
    uelt_assert_pointer_null("closure.value", closure.value);

    return NULL;
}

static void *uel_signal_execution(uel_closure_t *closure){
    bool *success = (bool *)closure->context;
    *success = true;

    return NULL;
}
static void *echo_param(uel_closure_t *closure){
    char *text = (char *)closure->params;
    return (void *)text;
}
static char *should_invoke_closure(){
    bool success = false;
    uel_closure_t signaler = uel_closure_create(uel_signal_execution, &success, NULL);
    uel_closure_invoke(&signaler, NULL);
    uelt_assert("uel_closure_invoke() must have had set success to true", success);

    uel_closure_t repeater = uel_closure_create(&echo_param, NULL, NULL);
    const char *string = "Peas, nuts and peanuts.";
    const char *value = uel_closure_invoke(&repeater, (void *)string);
    uelt_assert_pointers_equal("uel_closure_invoke(&repeater)", string, value);

    return NULL;
}

static void uel_signal_destruction(uel_closure_t *closure){
    bool *destroyed = (bool *)closure->context;
    *destroyed = true;
}
static char *should_destroy_closure(){
    bool destroyed = false;
    uel_closure_t closure = uel_closure_create(&nop1, &destroyed, uel_signal_destruction);
    uel_closure_invoke(&closure, NULL);
    uelt_assert_not(
        "uel_closure_invoke() must not have changed destroyed value",
        destroyed
    );
    uel_closure_destroy(&closure);
    uelt_assert("uel_closure_destroy() must have had set destroyed to true", destroyed);

    return NULL;
}

static void *add(uel_closure_t *closure){
    uint8_t a = (uint8_t)(uintptr_t)closure->context;
    uint8_t b = (uint8_t)(uintptr_t)closure->params;
    return (void *)(uintptr_t)(a + b);
}
static void *multiply(uel_closure_t *closure){
    uint8_t a = (uint8_t)(uintptr_t)closure->context;
    uint8_t b = (uint8_t)(uintptr_t)closure->params;
    return (void *)(uintptr_t)(a * b);
}
static char *should_check_uel_closure_return(){
    uel_closure_t plus_two = uel_closure_create(&add, (void *)2, NULL);
    uel_closure_t plus_three = uel_closure_create(&add, (void *)3, NULL);
    uel_closure_t times_two = uel_closure_create(&multiply, (void *)2, NULL);
    uel_closure_t times_three = uel_closure_create(&multiply, (void *)3, NULL);

    uint8_t res1 = (uint8_t)(uintptr_t)uel_closure_invoke(&plus_two, (void *)25);
    uint8_t res2 = (uint8_t)(uintptr_t)uel_closure_invoke(&plus_three, (void *)176);
    uint8_t res3 = (uint8_t)(uintptr_t)uel_closure_invoke(&times_two, (void *)17);
    uint8_t res4 = (uint8_t)(uintptr_t)uel_closure_invoke(&times_three, (void *)51);

    uelt_assert_ints_equal("25 + 2", 27, res1);
    uelt_assert_ints_equal("176 + 3", 179, res2);
    uelt_assert_ints_equal("17 * 2", 34, res3);
    uelt_assert_ints_equal("51 * 3", 153, res4);

    return NULL;
}

char * uel_closure_run_tests(){
    uelt_run_test("should correctly create closure", should_create_closure);
    uelt_run_test("should correctly invoke closure", should_invoke_closure);
    uelt_run_test("should correctly destroy closure", should_destroy_closure);
    uelt_run_test("should verify the closure's returned value", should_check_uel_closure_return);

    return NULL;
}
