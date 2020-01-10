#include "closure.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../minunit.h"
#include "utils/closure.h"

static void *nop1(closure_t *closure){ return NULL; }
static void nop2(closure_t *closure){}
static char *should_create_closure(){
    uint8_t context = 18;

    closure_t closure = closure_create(&nop1, &context, &nop2);
    mu_assert_pointers_equal("closure.function", closure.function, &nop1);
    mu_assert_pointers_equal("closure.context", closure.context, &context);
    mu_assert_pointers_equal("closure.destructor", closure.destructor, &nop2);
    mu_assert_pointer_null("closure.params", closure.params);
    mu_assert_pointer_null("closure.value", closure.value);

    return NULL;
}

static void *signal_execution(closure_t *closure){
    bool *success = (bool *)closure->context;
    *success = true;

    return NULL;
}
static void *echo_param(closure_t *closure){
    char *text = (char *)closure->params;
    return (void *)text;
}
static char *should_invoke_closure(){
    bool success = false;
    closure_t signaler = closure_create(&signal_execution, &success, NULL);
    closure_invoke(&signaler, NULL);
    mu_assert("closure_invoke() must have had set success to true", success);

    closure_t repeater = closure_create(&echo_param, NULL, NULL);
    const char *string = "Peas, nuts and peanuts.";
    const char *value = closure_invoke(&repeater, (void *)string);
    mu_assert_pointers_equal("closure_invoke(&repeater)", string, value);

    return NULL;
}

static void signal_destruction(closure_t *closure){
    bool *destroyed = (bool *)closure->context;
    *destroyed = true;
}
static char *should_destroy_closure(){
    bool destroyed = false;
    closure_t closure = closure_create(&nop1, &destroyed, &signal_destruction);
    closure_invoke(&closure, NULL);
    mu_assert_not(
        "closure_invoke() must not have changed destroyed value",
        destroyed
    );
    closure_destroy(&closure);
    mu_assert("closure_destroy() must have had set destroyed to true", destroyed);

    return NULL;
}

static void *add(closure_t *closure){
    uint8_t a = (uint8_t)(size_t)closure->context;
    uint8_t b = (uint8_t)(size_t)closure->params;
    return (void *)(size_t)(a + b);
}
static void *multiply(closure_t *closure){
    uint8_t a = (uint8_t)(size_t)closure->context;
    uint8_t b = (uint8_t)(size_t)closure->params;
    return (void *)(size_t)(a * b);
}
static char *should_check_closure_return(){
    closure_t plus_two = closure_create(&add, (void *)2, NULL);
    closure_t plus_three = closure_create(&add, (void *)3, NULL);
    closure_t times_two = closure_create(&multiply, (void *)2, NULL);
    closure_t times_three = closure_create(&multiply, (void *)3, NULL);

    uint8_t res1 = (uint8_t)(size_t)closure_invoke(&plus_two, (void *)25);
    uint8_t res2 = (uint8_t)(size_t)closure_invoke(&plus_three, (void *)176);
    uint8_t res3 = (uint8_t)(size_t)closure_invoke(&times_two, (void *)17);
    uint8_t res4 = (uint8_t)(size_t)closure_invoke(&times_three, (void *)51);

    mu_assert_ints_equal("25 + 2", 27, res1);
    mu_assert_ints_equal("176 + 3", 179, res2);
    mu_assert_ints_equal("17 * 2", 34, res3);
    mu_assert_ints_equal("51 * 3", 153, res4);

    return NULL;
}

char * closure_run_tests(){
    mu_run_test("should correctly create closure", should_create_closure);
    mu_run_test("should correctly invoke closure", should_invoke_closure);
    mu_run_test("should correctly destroy closure", should_destroy_closure);
    mu_run_test("should verify the closure's returned value", should_check_closure_return);

    return NULL;
}
