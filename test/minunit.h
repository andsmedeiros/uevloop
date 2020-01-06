/* built upon http://www.jera.com/techinfo/jtns/jtn002.html */
#include <stdlib.h>
#include <stdio.h>

#define mu_assert(message, test) do {                       \
    if (!(test)) return "Assertion failed: "message;        \
} while (0)

#define mu_assert_not(message, test) mu_assert(message, (!test))

#define mu_assert_equals(id, expected, supplied, format) do {                   \
    if(expected != supplied){                                                   \
        char *error = malloc(512 * sizeof(char));                               \
        const char *template =                                                  \
            "Assertion failed: on '%s', expected "format" and got "format".";   \
        snprintf(error, 512, template, id, expected, supplied);                 \
        return error;                                                           \
    }                                                                           \
} while(0)

#define mu_assert_not_equals(id, expected, supplied, format) do {               \
    if(expected == supplied){                                                   \
        char *error = malloc(512 * sizeof(char));                               \
        const char *template =                                                  \
            "Assertion failed: on '%s', expected to differ from "format".";     \
        snprintf(error, 512, template, id, expected, supplied);                 \
        return error;                                                           \
    }                                                                           \
}while(0)

#define mu_assert_pointers_equal(id, expected, supplied) do {                       \
    mu_assert_equals(id, (void *)(size_t)expected, (void *)(size_t)supplied, "%u"); \
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
        char *error = malloc(1024 * sizeof(char));      \
        const char *template = "@test: %s\n    %s";     \
        snprintf(error, 1024, template, id, message);   \
        return error;                                   \
    }                                                   \
} while(0)

#define mu_run_test_group(id, group) do {               \
    char *message = group();                            \
    groups_run++;                                       \
    if(message){                                        \
        char *error = malloc(2048 * sizeof(char));      \
        const char *template = "@group: %s\n%s";        \
        snprintf(error, 2048, template, id, message);   \
        return error;                                   \
    }                                                   \
} while(0)

extern unsigned int tests_run;
extern unsigned int groups_run;
