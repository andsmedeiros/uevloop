#include "system-queues.h"

#include <stdlib.h>

#include "system/containers/system-queues.h"
#include "../../minunit.h"

static char *should_init_sysqueues(){
    sysqueues_t queues;
    sysqueues_init(&queues);

    mu_assert_pointers_equal(
        "queues.event_queue.buffer",
        queues.event_queue_buffer,
        queues.event_queue.buffer
    );
    mu_assert_pointers_equal(
        "queues.schedule_queue.buffer",
        queues.schedule_queue_buffer,
        queues.schedule_queue.buffer
    );
    mu_assert_int_zero("queues.event_queue.count", queues.event_queue.count);
    mu_assert_int_zero("queues.schedule_queue.count", queues.schedule_queue.count);

    return NULL;
}

static void *nop(closure_t *closure){ return NULL; }
static char *should_manipulate_the_event_queue(){
    sysqueues_t queues;
    sysqueues_init(&queues);

    closure_t closure = closure_create(&nop, NULL, NULL);
    event_t events[3] = {
        { CLOSURE_EVENT, closure, false, { 0, 0 } },
        { TIMER_EVENT, closure, false, { 0, 0 } },
        { SIGNAL_EVENT, closure, false, { 0, 0 } }
    };

    sysqueues_enqueue_event(&queues, &events[0]);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        1,
        sysqueues_count_enqueued_events(&queues)
    );
    sysqueues_enqueue_event(&queues, &events[1]);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        2,
        sysqueues_count_enqueued_events(&queues)
    );
    sysqueues_enqueue_event(&queues, &events[2]);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        3,
        sysqueues_count_enqueued_events(&queues)
    );

    event_t *event;
    event = sysqueues_get_enqueued_event(&queues);
    mu_assert_pointer_not_null("event pointer", event);
    mu_assert_ints_equal("event type must be CLOSURE", CLOSURE_EVENT, event->type);    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        2,
        sysqueues_count_enqueued_events(&queues)
    );
    event = sysqueues_get_enqueued_event(&queues);
    mu_assert_pointer_not_null("event pointer", event);
    mu_assert_ints_equal("event type must be TIMER", TIMER_EVENT, event->type);
    mu_assert_ints_equal(
        "sysqueues_count_enqueued_events",
        1,
        sysqueues_count_enqueued_events(&queues)
    );
    event = sysqueues_get_enqueued_event(&queues);
    mu_assert_pointer_not_null("event pointer", event);
    mu_assert_ints_equal("event type must be SIGNAL", SIGNAL_EVENT, event->type);
    mu_assert_int_zero(
        "sysqueues_count_enqueued_events",
        sysqueues_count_enqueued_events(&queues)
    );

    return NULL;
}

static char *should_manipulate_the_schedule_queue(){
    sysqueues_t queues;
    sysqueues_init(&queues);

    closure_t closure = closure_create(&nop, NULL, NULL);
    event_t events[3] = {
        { CLOSURE_EVENT, closure, false, { 0, 0 } },
        { TIMER_EVENT, closure, false, { 0, 0 } },
        { SIGNAL_EVENT, closure, false, { 0, 0 } }
    };

    sysqueues_schedule_event(&queues, &events[2]);
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        1,
        sysqueues_count_scheduled_events(&queues)
    );
    sysqueues_schedule_event(&queues, &events[1]);
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        2,
        sysqueues_count_scheduled_events(&queues)
    );
    sysqueues_schedule_event(&queues, &events[0]);
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        3,
        sysqueues_count_scheduled_events(&queues)
    );

    event_t *event;
    event = sysqueues_get_scheduled_event(&queues);
    mu_assert_pointer_not_null("event pointer", event);
    mu_assert_ints_equal("event type must be SIGNAL", SIGNAL_EVENT, event->type);    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        2,
        sysqueues_count_scheduled_events(&queues)
    );
    event = sysqueues_get_scheduled_event(&queues);
    mu_assert_pointer_not_null("event pointer", event);
    mu_assert_ints_equal("event type must be TIMER", TIMER_EVENT, event->type);
    mu_assert_ints_equal(
        "sysqueues_count_scheduled_events",
        1,
        sysqueues_count_scheduled_events(&queues)
    );
    event = sysqueues_get_scheduled_event(&queues);
    mu_assert_pointer_not_null("event pointer", event);
    mu_assert_ints_equal("event type must be CLOSURE", CLOSURE_EVENT, event->type);
    mu_assert_int_zero(
        "sysqueues_count_scheduled_events",
        sysqueues_count_scheduled_events(&queues)
    );

    return NULL;
}

char *sysqueues_run_tests(){

    mu_run_test("should correctly initialise a new sysqueues", should_init_sysqueues);
    mu_run_test(
        "should correctly manipulate the event queue",
        should_manipulate_the_event_queue
    );
    mu_run_test(
        "should correctly manipulate the schedule queue",
        should_manipulate_the_schedule_queue
    );

    return NULL;
}
