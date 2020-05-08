#ifndef UELT_H
#define UELT_H

/* built upon http://www.jera.com/techinfo/jtns/jtn002.html */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define uelt_assert(message, test) do {                                           \
    test_context.assertions_run++;                                                           \
    if (!(test)) {                           \
        const char *template = "Assertion failed: "message"\n\n@%s:%d> %s\n";   \
        snprintf(test_context.assertion_error, 256, template, __FILE__, __LINE__, __func__);          \
        return test_context.assertion_error;                                                           \
    }                                                                           \
} while (0)

#define uelt_assert_not(message, test) uelt_assert(message, (!test))

#define uelt_assert_equals(id, expected, supplied, format) do {                   \
    test_context.assertions_run++;                                                           \
    if(expected != supplied){                            \
        const char *template =                                                  \
            "Assertion failed: on '%s', expected "format" and got "format".\n\n"\
            "@%s:%d> %s";                                                       \
        snprintf(test_context.assertion_error, 256, template, id, expected, supplied,                 \
            __FILE__, __LINE__, __func__);                                      \
        return test_context.assertion_error;                                                           \
    }                                                                           \
} while(0)

#define uelt_assert_not_equals(id, expected, supplied, format) do {               \
    test_context.assertions_run++;                                                           \
    if(expected == supplied){                            \
        const char *template =                                                  \
            "Assertion failed: on '%s', expected to differ from "format".\n\n"  \
            "@%s:%d> %s";                                                       \
        snprintf(test_context.assertion_error, 256, template, id, supplied,                           \
            __FILE__, __LINE__, __func__);                                      \
        return test_context.assertion_error;                                                           \
    }                                                                           \
}while(0)

#define uelt_assert_pointers_equal(id, expected, supplied) do {                       \
    uelt_assert_equals(id, (void *)(uintptr_t)expected, (void *)(uintptr_t)supplied, "%u"); \
} while(0)

#define uelt_assert_pointers_not_equal(id, expected, supplied) do {           \
    uelt_assert_not_equals(id, (void *)expected, (void *)supplied, "%u");     \
} while(0)

#define uelt_assert_pointer_null(id, pointer) do{     \
    uelt_assert_pointers_equal(id, NULL, pointer);    \
} while(0)

#define uelt_assert_pointer_not_null(id, pointer) do{     \
    uelt_assert_pointers_not_equal(id, NULL, pointer);    \
} while(0)

#define uelt_assert_ints_equal(id, expected, supplied) do {               \
    uelt_assert_equals(id, expected, supplied, "%d");                     \
} while(0)

#define uelt_assert_ints_not_equal(id, expected, supplied) do {               \
    uelt_assert_not_equals(id, expected, supplied, "%d");                     \
} while(0)

#define uelt_assert_int_zero(id, supplied) do {           \
    uelt_assert_ints_equal(id, 0, supplied);              \
} while(0)

#define uelt_assert_int_not_zero(id, supplied) do {           \
    uelt_assert_ints_not_equal(id, 0, supplied);              \
} while(0)

#define uelt_assert_str_blank(id, string) do {  \
    uelt_assert_ints_equal(id, 0, string[0]);   \
} while(0)

#define uelt_assert_str_not_blank(id, string) do {  \
    uelt_assert_ints_not_equal(id, 0, string[0]);   \
} while(0)

#define uelt_assert_strs_equal(id, expected, supplied) do {     \
    uelt_assert_ints_equal(id, 0, strcmp(expected, supplied));  \
} while(0)

#define uelt_run_test(id, test) do {                                        \
    char *message = test();                                                 \
    test_context.tests_run++;                                               \
    if(message){                                                            \
        const char *template = "@test: %s\n    %s";                         \
        snprintf(test_context.test_error, 512, template, id, message);      \
        return test_context.test_error;                                     \
    }                                                                       \
} while(0)

#define uelt_run_test_group(id, group) do {               \
    char *message = group();                            \
    test_context.groups_run++;                                       \
    if(message){                                        \
        const char *template = "@group: %s\n%s";        \
        snprintf(test_context.group_error, 1024, template, id, message);   \
        return test_context.group_error;                                   \
    }                                                   \
} while(0)

typedef struct {
    unsigned int tests_run;
    unsigned int groups_run;
    unsigned int assertions_run;
    char assertion_error[256];
    char test_error[512];
    char group_error[1024];

} uelt_context_t;

extern uelt_context_t test_context;
#define DEFAULT_TEST_CONTEXT {0, 0, 0, {0}, {0}, {0}}

#endif /* UELT_H */
