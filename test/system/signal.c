#include "signal.h"

#include <stdlib.h>
#include "utils/linked-list.h"
#include "system/signal.h"
#include "system/event-loop.h"
#include "system/containers/system-pools.h"
#include "system/containers/system-queues.h"
#include "../uelt.h"

enum test_signal_event {
    TEST_SIGNAL_EVENT_1 = 0,
    TEST_SIGNAL_EVENT_2,
    TEST_SIGNAL_EVENT_3,
    TEST_SIGNAL_EVENT_COUNT
};

#define DECLARE_SIGNAL_RELAY()                                                  \
    uel_syspools_t pools;                                                       \
    uel_syspools_init(&pools);                                                  \
    uel_sysqueues_t queues;                                                     \
    uel_sysqueues_init(&queues);                                                \
    uel_evloop_t loop;                                                          \
    uel_evloop_init(&loop, &pools, &queues);                                    \
    uel_llist_t relay_buffer[TEST_SIGNAL_EVENT_COUNT];                          \
    uel_signal_relay_t relay;                                                   \
    uel_signal_relay_init(                                                      \
        &relay,                                                                 \
        &pools,                                                                 \
        &queues,                                                                \
        relay_buffer,                                                           \
        TEST_SIGNAL_EVENT_COUNT                                                 \
    );

static char *should_init_signal_relay(){
    DECLARE_SIGNAL_RELAY();

    uelt_assert_pointers_equal(
        "relay.pools",
        &pools,
        relay.pools
    );
    uelt_assert_pointers_equal(
        "relay.queues",
        &queues,
        relay.queues
    );
    uelt_assert_pointers_equal(
        "relay.signal_vector",
        relay_buffer,
        relay.signal_vector
    );
    uelt_assert_ints_equal("relay.width", 3, relay.width);

    return NULL;
}

static void *nop(uel_closure_t *closure){ return NULL; }
static char *should_listen(){
    DECLARE_SIGNAL_RELAY();

    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);
    uel_signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);

    uelt_assert_ints_equal(
        "relay.signal_vector[0].count",
        1,
        relay.signal_vector[0].count
    );
    uelt_assert_int_zero(
        "relay.signal_vector[1].count",
        relay.signal_vector[1].count
    );

    uel_signal_listen(TEST_SIGNAL_EVENT_2, &relay, &closure);

    uelt_assert_ints_equal(
        "relay.signal_vector[0].count",
        1,
        relay.signal_vector[0].count
    );
    uelt_assert_ints_equal(
        "relay.signal_vector[1].count",
        1,
        relay.signal_vector[1].count
    );

    uel_signal_listen_once(TEST_SIGNAL_EVENT_3, &relay, &closure);
    uelt_assert_ints_equal(
        "relay.signal_vector[2].count",
        1,
        relay.signal_vector[2].count
    );

    uel_signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);
    uelt_assert_ints_equal(
        "relay.signal_vector[0].count",
        2,
        relay.signal_vector[0].count
    );

    uel_signal_listen_once(TEST_SIGNAL_EVENT_1, &relay, &closure);
    uelt_assert_ints_equal(
        "relay.signal_vector[0].count",
        3,
        relay.signal_vector[0].count
    );

    return NULL;
}

static char *should_unlisten(){
    DECLARE_SIGNAL_RELAY();

    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);
    uel_signal_listener_t listener1 =
        uel_signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);
    uel_signal_listener_t listener2 =
        uel_signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);
    uel_signal_listener_t listener3 =
        uel_signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure);

    uelt_assert_ints_equal(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        3,
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );
    uelt_assert_not("listener2->unlistened", listener2->unlistened);

    uel_signal_unlisten(listener2);
    uelt_assert("listener2->unlistened", listener2->unlistened);
    uel_signal_emit(TEST_SIGNAL_EVENT_1, &relay, NULL);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        2,
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );

    uelt_assert_not("listener3->unlistened", listener3->unlistened);
    uel_signal_unlisten(listener3);
    uelt_assert("listener3->unlistened", listener3->unlistened);
    uel_signal_emit(TEST_SIGNAL_EVENT_1, &relay, NULL);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        1,
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );

    uelt_assert_not("listener1->unlistened", listener1->unlistened);
    uel_signal_unlisten(listener1);
    uelt_assert("listener1->unlistened", listener1->unlistened);
    uel_signal_emit(TEST_SIGNAL_EVENT_1, &relay, NULL);
    uel_evloop_run(&loop);
    uelt_assert_int_zero(
        "relay.signal_vector[TEST_SIGNAL_EVENT_1].count",
        relay.signal_vector[TEST_SIGNAL_EVENT_1].count
    );

    return NULL;
}


static void *increment(uel_closure_t *closure){
    uintptr_t *counter = (uintptr_t *)closure->context;
    uintptr_t amount = (uintptr_t)closure->params;

    *counter += amount;

    return NULL;
}
static char *should_emit(){
    DECLARE_SIGNAL_RELAY();

    uintptr_t counter1 = 0, counter2 = 0, counter3 = 0;

    uel_closure_t closure1 = uel_closure_create(&increment, &counter1, NULL);
    uel_closure_t closure2 = uel_closure_create(&increment, &counter2, NULL);
    uel_closure_t closure3 = uel_closure_create(&increment, &counter3, NULL);
    uel_signal_listen(TEST_SIGNAL_EVENT_1, &relay, &closure1);
    uel_signal_listen(TEST_SIGNAL_EVENT_2, &relay, &closure2);
    uel_signal_listen_once(TEST_SIGNAL_EVENT_3, &relay, &closure3);

    uel_signal_emit(TEST_SIGNAL_EVENT_1, &relay, (void *)1);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal("counter1", 1, counter1);

    uel_signal_emit(TEST_SIGNAL_EVENT_1, &relay, (void *)2);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal("counter1", 3, counter1);

    uel_signal_emit(TEST_SIGNAL_EVENT_2, &relay, (void *)2);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal("counter2", 2, counter2);

    uel_signal_emit(TEST_SIGNAL_EVENT_2, &relay, (void *)3);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal("counter2", 5, counter2);

    uel_signal_emit(TEST_SIGNAL_EVENT_3, &relay, (void *)3);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal("counter3", 3, counter3);

    uel_signal_emit(TEST_SIGNAL_EVENT_3, &relay, (void *)4);
    uel_evloop_run(&loop);
    uelt_assert_ints_equal("counter3", 3, counter3);

//    uel_signal_emit_urgent(TEST_SIGNAL_EVENT_2, &relay, (void *)3);
//    uelt_assert_ints_equal("counter2", 8, counter2);

    return NULL;
}

char *uel_signal_run_tests(){
    uelt_run_test(
        "should correctly initialise a signal relay",
        should_init_signal_relay
    );
    uelt_run_test(
        "should correctly listen on different signals",
        should_listen
    );
    uelt_run_test(
        "should correctly unlisten registered signals",
        should_unlisten
    );
    uelt_run_test(
        "should correctly emit diferent signals",
        should_emit
    );

    return NULL;
}
