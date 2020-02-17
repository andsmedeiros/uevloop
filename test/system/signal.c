#include "signal.h"

#include <stdlib.h>
#include "utils/linked-list.h"
#include "system/signal.h"
#include "system/event-loop.h"
#include "system/containers/system-pools.h"
#include "system/containers/system-queues.h"
#include "../minunit.h"

enum test_signal_event {
    TEST_SIGNAL_EVENT_1 = 0,
    TEST_SIGNAL_EVENT_2,
    TEST_SIGNAL_EVENT_3,
    TEST_SIGNAL_EVENT_COUNT
};

#define DECLARE_SIGNAL_RELAY()                                                  \
    syspools_t pools;                                                           \
    syspools_init(&pools);                                                      \
    sysqueues_t queues;                                                         \
    sysqueues_init(&queues);                                                    \
    evloop_t loop;                                                              \
    evloop_init(&loop, &pools, &queues);                                        \
    llist_t relay_buffer[TEST_SIGNAL_EVENT_COUNT];                              \
    signal_relay_t relay;                                                       \
    signal_relay_init(                                                          \
        &relay,                                                                 \
        &pools,                                                                 \
        &queues,                                                                \
        relay_buffer,                                                           \
        TEST_SIGNAL_EVENT_COUNT                                                 \
    );

static char *should_init_signal_relay(){
    DECLARE_SIGNAL_RELAY();

    mu_assert_pointers_equal(
        "relay.pools",
        &pools,
        relay.pools
    );
    mu_assert_pointers_equal(
        "relay.queues",
        &queues,
        relay.queues
    );
    mu_assert_pointers_equal(
        "relay.signal_vector",
        relay_buffer,
        relay.signal_vector
    );
    mu_assert_ints_equal("relay.width", 3, relay.width);

    return NULL;
}

static void *nop(closure_t *closure){ return NULL; }
static char *should_listen(){
    DECLARE_SIGNAL_RELAY();

    closure_t closure = closure_create(&nop, NULL, NULL);
    signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);

    mu_assert_ints_equal(
        "relay.signal_vector[0].count",
        1,
        relay.signal_vector[0].count
    );
    mu_assert_int_zero(
        "relay.signal_vector[1].count",
        relay.signal_vector[1].count
    );

    signal_listen(TEST_SIGNAL_EVENT_2, &relay, &closure);

    mu_assert_ints_equal(
        "relay.signal_vector[0].count",
        1,
        relay.signal_vector[0].count
    );
    mu_assert_ints_equal(
        "relay.signal_vector[1].count",
        1,
        relay.signal_vector[1].count
    );

    signal_listen_once(TEST_SIGNAL_EVENT_3, &relay, &closure);
    mu_assert_ints_equal(
        "relay.signal_vector[2].count",
        1,
        relay.signal_vector[2].count
    );

    signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);
    mu_assert_ints_equal(
        "relay.signal_vector[0].count",
        2,
        relay.signal_vector[0].count
    );

    signal_listen_once(TEST_SIGNAL_EVENT_1, &relay, &closure);
    mu_assert_ints_equal(
        "relay.signal_vector[0].count",
        3,
        relay.signal_vector[0].count
    );

    return NULL;
}

static char *should_unlisten(){
    DECLARE_SIGNAL_RELAY();

    closure_t closure = closure_create(&nop, NULL, NULL);
    signal_listener_t listener1 =
        signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);
    signal_listener_t listener2 =
        signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);
    signal_listener_t listener3 =
        signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);

    mu_assert_ints_equal(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        3,
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );
    mu_assert_not("listener2->unlistened", listener2->unlistened);

    signal_unlisten(listener2);
    mu_assert("listener2->unlistened", listener2->unlistened);
    signal_emit(TEST_SIGNAL_EVENT_1, &relay, NULL);
    evloop_run(&loop);
    mu_assert_ints_equal(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        2,
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );

    mu_assert_not("listener3->unlistened", listener3->unlistened);
    signal_unlisten(listener3);
    mu_assert("listener3->unlistened", listener3->unlistened);
    signal_emit(TEST_SIGNAL_EVENT_1, &relay, NULL);
    evloop_run(&loop);
    mu_assert_ints_equal(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        1,
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );

    mu_assert_not("listener1->unlistened", listener1->unlistened);
    signal_unlisten(listener1);
    mu_assert("listener1->unlistened", listener1->unlistened);
    signal_emit(TEST_SIGNAL_EVENT_1, &relay, NULL);
    evloop_run(&loop);
    mu_assert_int_zero(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );

    return NULL;
}


static void *increment(closure_t *closure){
    uintptr_t *counter = (uintptr_t *)closure->context;
    uintptr_t amount = (uintptr_t)closure->params;

    *counter += amount;

    return NULL;
}
static char *should_emit(){
    DECLARE_SIGNAL_RELAY();

    uintptr_t counter1 = 0, counter2 = 0, counter3 = 0;

    closure_t closure1 = closure_create(&increment, &counter1, NULL);
    closure_t closure2 = closure_create(&increment, &counter2, NULL);
    closure_t closure3 = closure_create(&increment, &counter3, NULL);
    signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure1);
    signal_listen(TEST_SIGNAL_EVENT_2, &relay, &closure2);
    signal_listen_once(TEST_SIGNAL_EVENT_3, &relay, &closure3);

    signal_emit(TEST_SIGNAL_EVENT_1, &relay, (void *)1);
    evloop_run(&loop);
    mu_assert_ints_equal("counter1", 1, counter1);

    signal_emit(TEST_SIGNAL_EVENT_1, &relay, (void *)2);
    evloop_run(&loop);
    mu_assert_ints_equal("counter1", 3, counter1);

    signal_emit(TEST_SIGNAL_EVENT_2, &relay, (void *)2);
    evloop_run(&loop);
    mu_assert_ints_equal("counter2", 2, counter2);

    signal_emit(TEST_SIGNAL_EVENT_2, &relay, (void *)3);
    evloop_run(&loop);
    mu_assert_ints_equal("counter2", 5, counter2);

    signal_emit(TEST_SIGNAL_EVENT_3, &relay, (void *)3);
    evloop_run(&loop);
    mu_assert_ints_equal("counter3", 3, counter3);

    signal_emit(TEST_SIGNAL_EVENT_3, &relay, (void *)4);
    evloop_run(&loop);
    mu_assert_ints_equal("counter3", 3, counter3);


    return NULL;
}

char *signal_run_tests(){
    mu_run_test(
        "should correctly initialise a signal relay",
        should_init_signal_relay
    );
    mu_run_test(
        "should correctly listen on different signals",
        should_listen
    );
    mu_run_test(
        "should correctly unlisten registered signals",
        should_unlisten
    );
    mu_run_test(
        "should correctly emit diferent signals",
        should_emit
    );

    return NULL;
}
