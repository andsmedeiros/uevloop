#include "scheduler.h"

#include <stdlib.h>

#include "uevloop/utils/closure.h"
#include "uevloop/system/containers/system-pools.h"
#include "uevloop/system/containers/system-queues.h"
#include "uevloop/system/scheduler.h"
#include "uevloop/system/event-loop.h"
#include "../uelt.h"

#define DECLARE_SCHEDULER()                                                    \
    uel_syspools_t pools;                                                      \
    uel_syspools_init(&pools);                                                 \
    uel_sysqueues_t queues;                                                    \
    uel_sysqueues_init(&queues);                                               \
    uel_scheduer_t scheduler;                                                  \
    uel_sch_init(&scheduler, &pools, &queues);

static char *should_init_scheduler(){
    DECLARE_SCHEDULER();

    uelt_assert_pointers_equal(
        "scheduler.pools",
        scheduler.pools,
        &pools
    );
    uelt_assert_pointers_equal(
        "scheduler.queues",
        scheduler.queues,
        &queues
    );
    uelt_assert_pointers_equal(
        "scheduler.timer_list.head",
        scheduler.timer_list.tail,
        scheduler.timer_list.head
    );
    uelt_assert_int_zero("scheduler.timer_list.count", scheduler.timer_list.count);
    uelt_assert_pointers_equal(
        "scheduler.pause_list.head",
        scheduler.pause_list.tail,
        scheduler.pause_list.head
    );
    uelt_assert_int_zero("scheduler.pause_list.count", scheduler.pause_list.count);
    uelt_assert_int_zero("scheduler.timer", scheduler.timer);

    return NULL;
}

static void *nop(uel_closure_t *closure){ return NULL; }
static char *should_schedule_for_later_execution(){
    DECLARE_SCHEDULER();

    uel_sch_run_later(&scheduler, 1000, uel_closure_create(&nop, NULL, NULL));
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(&queues)
    );
    uel_sch_manage_timers(&scheduler);
    uelt_assert_ints_equal(
        "scheduler.timer_list.count",
        1,
        scheduler.timer_list.count
    );

    uel_event_t *event;
    event = (uel_event_t *)scheduler.timer_list.tail->value;
    uelt_assert_ints_equal(
        "scheduler.timer_list.tail->value->detail.timer.timeout",
        1000,
        event->detail.timer.timeout
    );
    uelt_assert_pointers_equal(
        "scheduler.timer_list.tail->value->closure.function",
        &nop,
        event->closure.function
    );

    uel_sch_run_later(&scheduler, 500, uel_closure_create(&nop, NULL, NULL));
    uelt_assert_ints_equal(
        "uel_sysqueues_count_scheduled_events",
        1,
        uel_sysqueues_count_scheduled_events(&queues)
    );
    uel_sch_manage_timers(&scheduler);
    uelt_assert_ints_equal(
        "scheduler.timer_list.count",
        2,
        scheduler.timer_list.count
    );

    event = (uel_event_t *)scheduler.timer_list.tail->value;
    uelt_assert_ints_equal(
        "scheduler.timer_list.tail->value->detail.timer.timeout",
        500,
        event->detail.timer.timeout
    );

    return NULL;
}

static char *should_schedule_intervals(){
    DECLARE_SCHEDULER();
    uel_closure_t closure = uel_closure_create(&nop, NULL, NULL);

    {
        uel_sch_run_at_intervals(&scheduler, 1000, true, closure);
        uelt_assert_ints_equal(
            "uel_sysqueues_count_enqueued_events",
            1,
            uel_sysqueues_count_enqueued_events(&queues)
        );
        uelt_assert_int_zero("scheduler.timer_list.count", scheduler.timer_list.count);
        uel_event_t *event = uel_cqueue_peek_tail(&scheduler.queues->event_queue);
        uelt_assert_ints_equal(
            "timeout at system's event queue tail element",
            1000,
            event->detail.timer.timeout
        );
        uelt_assert(
            "`repeating` at the system's event queue tail element must had been set",
            event->repeating
        );

    }
    {
        uel_sch_run_at_intervals(&scheduler, 500, false, closure);
        uelt_assert_ints_equal(
            "uel_sysqueues_count_scheduled_events(&queues)",
            1,
            uel_sysqueues_count_scheduled_events(&queues)
        );
        uel_sch_manage_timers(&scheduler);
        uelt_assert_ints_equal(
            "scheduler.timer_list.count",
            1,
            scheduler.timer_list.count
        );
        uel_llist_node_t *node = (uel_llist_node_t *)scheduler.timer_list.tail;
        uel_event_t *event = (uel_event_t *)node->value;
        uelt_assert_ints_equal(
            "scheduler.timer_list.tail->detail.timer.timeout",
            500,
            event->detail.timer.timeout
        );
    }
    {
        uel_sch_run_at_intervals(&scheduler, 200, false, closure);
        uelt_assert_ints_equal(
            "uel_sysqueues_count_scheduled_events(&queues)",
            1,
            uel_sysqueues_count_scheduled_events(&queues)
        );
        uel_sch_manage_timers(&scheduler);
        uelt_assert_ints_equal(
            "scheduler.timer_list.count",
            2,
            scheduler.timer_list.count
        );
        uel_llist_node_t *node = (uel_llist_node_t *)scheduler.timer_list.tail;
        uel_event_t *previous = (uel_event_t *)node->value;
        uel_event_t *current = (uel_event_t *)node->next->value;
        uelt_assert_ints_equal(
            "scheduler.timer_list.tail->detail.timer.timeout",
            200,
            previous->detail.timer.timeout
        );
        uelt_assert_ints_equal(
            "scheduler.timer_list.tail->next->detail.timer.timeout",
            500,
            current->detail.timer.timeout
        );
    }


    return NULL;
}

static void fast_forward(uel_scheduer_t *scheduler, uint32_t *timer, uint32_t amount){
    *timer += amount;
    uel_sch_update_timer(scheduler, *timer);
}
static void operate(uel_scheduer_t *scheduler, uel_evloop_t *loop){
    uel_sch_manage_timers(scheduler);
    uel_evloop_run(loop);
}
static void *uel_signal_execution(uel_closure_t *closure){
    bool *executed = (bool *)closure->context;
    *executed = true;
    return NULL;
}
static char *should_operate(){
    DECLARE_SCHEDULER();

    uint32_t timer = 0;
    uel_evloop_t loop;
    uel_evloop_init(&loop, &pools, &queues);

    fast_forward(&scheduler, &timer, 1);
    uelt_assert_ints_equal("scheduler.timer", 1, scheduler.timer);

    bool success1 = false, success2 = false;

    uel_sch_run_later(
        &scheduler,
        3,
        uel_closure_create(uel_signal_execution, (void *)&success1, NULL)
    );
    uel_sch_run_at_intervals(
        &scheduler,
        5,
        true,
        uel_closure_create(uel_signal_execution, (void *)&success2, NULL)
    );
    uelt_assert_not("success1 must had been unset", success1);
    uelt_assert_not("success2 must had been unset", success2);

    operate(&scheduler, &loop);
    uelt_assert_not("success1 must had been unset", success1);
    uelt_assert("success2 must had been set", success2);
    success2 = false;

    fast_forward(&scheduler, &timer, 3);

    operate(&scheduler, &loop);
    uelt_assert("success1 must had been set", success1);
    uelt_assert_not("success2 must had been unset", success2);
    success1 = false;

    fast_forward(&scheduler, &timer, 2);

    operate(&scheduler, &loop);
    uelt_assert_not("success1 must had been unset", success1);
    uelt_assert("success2 must had been set", success2);
    success2 = false;

    fast_forward(&scheduler, &timer, 1);
    operate(&scheduler, &loop);
    uelt_assert_not("success1 must had been unset", success1);
    uelt_assert_not("success2 must had been unset", success2);

    fast_forward(&scheduler, &timer, 4);

    operate(&scheduler, &loop);
    uelt_assert_not("success1 must had been unset", success1);
    uelt_assert("success2 must had been set", success2);

    return NULL;
}

static char *should_handle_timer_statuses(){
    DECLARE_SCHEDULER();
    uint32_t counter = 0;

    uel_closure_t do_nothing = uel_closure_create(nop, NULL, NULL);

    uel_event_t *timer =
        uel_sch_run_at_intervals(&scheduler, 10, false, do_nothing);
    uel_sch_manage_timers(&scheduler);

    uelt_assert_ints_equal(
        "scheduler.timer_list.count",
        1,
        scheduler.timer_list.count
    );
    uelt_assert_int_zero("scheduler.pause_list.count", scheduler.pause_list.count);

    uel_event_timer_pause(timer);
    uelt_assert_ints_equal(
        "scheduler.timer_list.count",
        1,
        scheduler.timer_list.count
    );
    uelt_assert_int_zero("scheduler.pause_list.count", scheduler.pause_list.count);

    fast_forward(&scheduler, &counter, 9);
    uel_sch_manage_timers(&scheduler);
    uelt_assert_ints_equal(
        "scheduler.timer_list.count",
        1,
        scheduler.timer_list.count
    );
    uelt_assert_int_zero("scheduler.pause_list.count", scheduler.pause_list.count);

    fast_forward(&scheduler, &counter, 1);
    uel_sch_manage_timers(&scheduler);
    uelt_assert_int_zero(
        "scheduler.timer_list.count",
        scheduler.timer_list.count
    );
    uelt_assert_ints_equal(
        "scheduler.pause_list.count",
        1,
        scheduler.pause_list.count
    );

    fast_forward(&scheduler, &counter, 11);
    uel_sch_manage_timers(&scheduler);
    uelt_assert_int_zero(
        "scheduler.timer_list.count",
        scheduler.timer_list.count
    );
    uelt_assert_ints_equal(
        "scheduler.pause_list.count",
        1,
        scheduler.pause_list.count
    );

    uel_event_timer_resume(timer);
    uel_sch_manage_timers(&scheduler);
    uelt_assert_ints_equal(
        "scheduler.timer_list.count",
        1,
        scheduler.timer_list.count
    );
    uelt_assert_int_zero("scheduler.pause_list.count", scheduler.pause_list.count);

    uel_event_timer_cancel(timer);
    fast_forward(&scheduler, &counter, 10);
    uel_sch_manage_timers(&scheduler);
    uelt_assert_ints_equal(
        "scheduler.queues->event_queue.count",
        1,
        scheduler.queues->event_queue.count
    );
    uelt_assert_int_zero("scheduler.timer_list.count", scheduler.timer_list.count);

    return NULL;
}

char *sch_run_tests(){
    uelt_run_test("should correctly initialise an scheduler", should_init_scheduler);
    uelt_run_test(
        "should correctly schedule events for later execution",
        should_schedule_for_later_execution
    );
    uelt_run_test(
        "should correctly schedule interval timers in both immediate and delayed modes",
        should_schedule_intervals
    );
    uelt_run_test(
        "should correctly handle timer events in different statuses",
        should_handle_timer_statuses
    );
    uelt_run_test(
        "should correctly process events as they are input and run them when managing",
        should_operate
    );
    return NULL;
}
