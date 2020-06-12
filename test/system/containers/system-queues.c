#include "system-queues.h"

#include <stdlib.h>

#include "uevloop/system/containers/system-queues.h"
#include "../../uelt.h"

static char *should_init_sysqueues(){
    uel_sysqueues_t queues;
    uel_sysqueues_init(&queues);

    uelt_assert_pointers_equal(
        "queues.event_queue.buffer",
        queues.event_queue_buffer,
        queues.event_queue.buffer
    );
    uelt_assert_pointers_equal(
        "queues.schedule_queue.buffer",
        queues.schedule_queue_buffer,
        queues.schedule_queue.buffer
    );
    uelt_assert_int_zero("queues.event_queue.count", queues.event_queue.count);
    uelt_assert_int_zero("queues.schedule_queue.count", queues.schedule_queue.count);

    return NULL;
}

static void *nop(void *context, void *params){ return NULL; }
static char *should_manipulate_the_event_queue(){
    uel_sysqueues_t queues;
    uel_sysqueues_init(&queues);

    uel_closure_t closure = uel_closure_create(&nop, NULL);
    uel_event_t events[3];
    uel_event_config_closure(&events[0], &closure, (void *)&queues, false);
    uel_event_config_timer(&events[1], 10, false, false, &closure, (void *)&queues, 0);
    uel_event_config_signal(&events[2], 0, NULL, NULL);

    uel_sysqueues_enqueue_event(&queues, &events[0]);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(&queues)
    );
    uel_sysqueues_enqueue_event(&queues, &events[1]);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        2,
        uel_sysqueues_count_enqueued_events(&queues)
    );
    uel_sysqueues_enqueue_event(&queues, &events[2]);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        3,
        uel_sysqueues_count_enqueued_events(&queues)
    );

    uel_event_t *event;
    event = uel_sysqueues_get_enqueued_event(&queues);
    uelt_assert_pointer_not_null("event pointer", event);
    uelt_assert_ints_equal("event type must be CLOSURE", UEL_CLOSURE_EVENT, event->type);    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        2,
        uel_sysqueues_count_enqueued_events(&queues)
    );
    event = uel_sysqueues_get_enqueued_event(&queues);
    uelt_assert_pointer_not_null("event pointer", event);
    uelt_assert_ints_equal("event type must be TIMER", UEL_TIMER_EVENT, event->type);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_enqueued_events",
        1,
        uel_sysqueues_count_enqueued_events(&queues)
    );
    event = uel_sysqueues_get_enqueued_event(&queues);
    uelt_assert_pointer_not_null("event pointer", event);
    uelt_assert_ints_equal("event type must be SIGNAL", UEL_SIGNAL_EVENT, event->type);
    uelt_assert_int_zero(
        "uel_sysqueues_count_enqueued_events",
        uel_sysqueues_count_enqueued_events(&queues)
    );

    return NULL;
}

static char *should_manipulate_the_schedule_queue(){
    uel_sysqueues_t queues;
    uel_sysqueues_init(&queues);

    uel_closure_t closure = uel_closure_create(&nop, NULL);
    uel_event_t events[3];
    uel_event_config_closure(&events[0], &closure, (void *)&queues, false);
    uel_event_config_timer(&events[1], 10, false, false, &closure, (void *)&queues, 0);
    uel_event_config_signal(&events[2], 0, NULL, NULL);

    uel_sysqueues_schedule_event(&queues, &events[2]);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(&queues)
    );
    uel_sysqueues_schedule_event(&queues, &events[1]);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        2,
        uel_sysqueues_count_scheduled_events(&queues)
    );
    uel_sysqueues_schedule_event(&queues, &events[0]);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        3,
        uel_sysqueues_count_scheduled_events(&queues)
    );

    uel_event_t *event;
    event = uel_sysqueues_get_scheduled_event(&queues);
    uelt_assert_pointer_not_null("event pointer", event);
    uelt_assert_ints_equal("event type must be SIGNAL", UEL_SIGNAL_EVENT, event->type);    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        2,
        uel_sysqueues_count_scheduled_events(&queues)
    );
    event = uel_sysqueues_get_scheduled_event(&queues);
    uelt_assert_pointer_not_null("event pointer", event);
    uelt_assert_ints_equal("event type must be TIMER", UEL_TIMER_EVENT, event->type);
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(&queues)
    );
    event = uel_sysqueues_get_scheduled_event(&queues);
    uelt_assert_pointer_not_null("event pointer", event);
    uelt_assert_ints_equal("event type must be CLOSURE", UEL_CLOSURE_EVENT, event->type);
    uelt_assert_int_zero(
        "uel_sysqueues_count_scheduled_events",
        uel_sysqueues_count_scheduled_events(&queues)
    );

    return NULL;
}

char *uel_sysqueues_run_tests(){

    uelt_run_test("should correctly initialise a new sysqueues", should_init_sysqueues);
    uelt_run_test(
        "should correctly manipulate the event queue",
        should_manipulate_the_event_queue
    );
    uelt_run_test(
        "should correctly manipulate the schedule queue",
        should_manipulate_the_schedule_queue
    );

    return NULL;
}
