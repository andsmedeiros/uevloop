#include "closure.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../uelt.h"
#include "uevloop/utils/closure.h"

static void *nop1(void *context, void *params){ return NULL; }
static char *should_create_closure(){
    uint8_t context = 18;

    uel_closure_t closure = uel_closure_create(&nop1, &context);
    uelt_assert_pointers_equal("closure.function", closure.function, &nop1);
    uelt_assert_pointers_equal("closure.context", closure.context, &context);

    return NULL;
}

static void *uel_signal_execution(void *context, void *params){
    bool *success = (bool *)context;
    *success = true;

    return NULL;
}
static void *echo_param(void *context, void *params){
    char *text = (char *)params;
    return (void *)text;
}
static char *should_invoke_closure(){
    bool success = false;
    uel_closure_t signaler = uel_closure_create(uel_signal_execution, &success);
    uel_closure_invoke(&signaler, NULL);
    uelt_assert("uel_closure_invoke() must have had set success to true", success);

    uel_closure_t repeater = uel_closure_create(&echo_param, NULL);
    const char *string = "Peas, nuts and peanuts.";
    const char *value = uel_closure_invoke(&repeater, (void *)string);
    uelt_assert_pointers_equal("uel_closure_invoke(&repeater)", string, value);

    return NULL;
}

static void *add(void *context, void *params){
    uint8_t a = (uint8_t)(uintptr_t)context;
    uint8_t b = (uint8_t)(uintptr_t)params;
    return (void *)(uintptr_t)(a + b);
}
static void *multiply(void *context, void *params){
    uint8_t a = (uint8_t)(uintptr_t)context;
    uint8_t b = (uint8_t)(uintptr_t)params;
    return (void *)(uintptr_t)(a * b);
}
static char *should_check_uel_closure_return(){
    uel_closure_t plus_two = uel_closure_create(&add, (void *)2);
    uel_closure_t plus_three = uel_closure_create(&add, (void *)3);
    uel_closure_t times_two = uel_closure_create(&multiply, (void *)2);
    uel_closure_t times_three = uel_closure_create(&multiply, (void *)3);

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

static char *should_create_nop() {
    uel_closure_t nop = uel_nop();
    uelt_assert_pointer_not_null("nop.function", (uintptr_t)nop.function);
    uelt_assert_pointer_null("nop.context", nop.context);

    void *arg = NULL, *value = (void *)1;
    value = uel_closure_invoke(&nop, arg);
    uelt_assert_pointer_null("arg", arg);
    uelt_assert_pointer_null("value", value);

    return NULL;
}

char * uel_closure_run_tests(){
    uelt_run_test("should correctly create closure", should_create_closure);
    uelt_run_test("should correctly invoke closure", should_invoke_closure);
    uelt_run_test("should verify the closure's returned value", should_check_uel_closure_return);
    uelt_run_test("should create a nop closure", should_create_nop);

    return NULL;
}
