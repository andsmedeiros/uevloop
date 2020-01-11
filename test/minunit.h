/* built upon http://www.jera.com/techinfo/jtns/jtn002.html */
#include <stdlib.h>
#include <stdio.h>

#define mu_assert(message, test) do {                                           \
    assertions_run++;                                                           \
    if (!(test)) {                                                              \
        char *error = malloc(1024 * sizeof(char));                              \
        const char *template = "Assertion failed: "message"\n\n@%s:%d> %s\n";   \
        snprintf(error, 1024, template, __FILE__, __LINE__, __func__);          \
        return error;                                                           \
    }                                                                           \
} while (0)

#define mu_assert_not(message, test) mu_assert(message, (!test))

#define mu_assert_equals(id, expected, supplied, format) do {                   \
    assertions_run++;                                                           \
    if(expected != supplied){                                                   \
        char *error = malloc(1024 * sizeof(char));                              \
        const char *template =                                                  \
            "Assertion failed: on '%s', expected "format" and got "format".\n\n"\
            "@%s:%d> %s";                                                       \
        snprintf(error, 1024, template, id, expected, supplied,                 \
            __FILE__, __LINE__, __func__);                                      \
        return error;                                                           \
    }                                                                           \
} while(0)

#define mu_assert_not_equals(id, expected, supplied, format) do {               \
    assertions_run++;                                                           \
    if(expected == supplied){                                                   \
        char *error = malloc(1024 * sizeof(char));                              \
        const char *template =                                                  \
            "Assertion failed: on '%s', expected to differ from "format".\n\n"  \
            "@%s:%d> %s";                                                       \
        snprintf(error, 1024, template, id, supplied,                           \
            __FILE__, __LINE__, __func__);                                      \
        return error;                                                           \
    }                                                                           \
}while(0)

#define mu_assert_pointers_equal(id, expected, supplied) do {                       \
    mu_assert_equals(id, (void *)(uintptr_t)expected, (void *)(uintptr_t)supplied, "%u"); \
} while(0)

#define mu_assert_pointers_not_equal(id, expected, supplied) do {           \
    mu_assert_not_equals(id, (void *)expected, (void *)supplied, "%u");     \
} while(0)

#define mu_assert_pointer_null(id, pointer) do{     \
    mu_assert_pointers_equal(id, NULL, pointer);    \
} while(0)

#define mu_assert_pointer_not_null(id, pointer) do{     \
    mu_assert_pointers_not_equal(id, NULL, pointer);    \
} while(0)

#define mu_assert_ints_equal(id, expected, supplied) do {               \
    mu_assert_equals(id, expected, supplied, "%d");                     \
} while(0)

#define mu_assert_ints_not_equal(id, expected, supplied) do {               \
    mu_assert_not_equals(id, expected, supplied, "%d");                     \
} while(0)

#define mu_assert_int_zero(id, supplied) do {           \
    mu_assert_ints_equal(id, 0, supplied);              \
} while(0)

#define mu_assert_int_not_zero(id, supplied) do {           \
    mu_assert_ints_not_equal(id, 0, supplied);              \
} while(0)

#define mu_run_test(id, test) do {                      \
    char *message = test();                             \
    tests_run++;                                        \
    if(message){                                        \
        char *error = malloc(2048 * sizeof(char));      \
        const char *template = "@test: %s\n    %s";     \
        snprintf(error, 2048, template, id, message);   \
        return error;                                   \
    }                                                   \
} while(0)

#define mu_run_test_group(id, group) do {               \
    char *message = group();                            \
    groups_run++;                                       \
    if(message){                                        \
        char *error = malloc(4096 * sizeof(char));      \
        const char *template = "@group: %s\n%s";        \
        snprintf(error, 4096, template, id, message);   \
        return error;                                   \
    }                                                   \
} while(0)

extern unsigned int tests_run;
extern unsigned int groups_run;
extern unsigned int assertions_run;
