#include "scheduler.h"

#include <stdlib.h>

#include "utils/closure.h"
#include "system/syspools.h"
#include "system/scheduler.h"
#include "system/event-loop.h"
#include "../minunit.h"

#define DECLARE_SCHEDULER()                                                     \
    syspools_t pools;                                                              \
    void *event_queue_buffer[8];                                                \
    cqueue_t event_queue;                                                       \
    void *reschedule_queue_buffer[8];                                           \
    cqueue_t reschedule_queue;                                                  \
    syspools_init(&pools);                                                         \
    cqueue_init(&event_queue, event_queue_buffer, 3);                           \
    cqueue_init(&reschedule_queue, reschedule_queue_buffer, 3 );                \
    scheduler_t scheduler;                                                      \
    sch_init(                                                                   \
        &scheduler,                                                             \
        &pools,                                                 \
        &event_queue,                                                           \
        &reschedule_queue                                                       \
    );

static char *should_init_scheduler(){
    DECLARE_SCHEDULER();

    mu_assert_pointers_equal(
        "scheduler.pools",
        scheduler.pools,
        &pools
    );
    mu_assert_pointers_equal(
        "scheduler.reschedule_queue",
        scheduler.reschedule_queue,
        &reschedule_queue
    );
    mu_assert_pointers_equal(
        "scheduler.reschedule_queue",
        scheduler.reschedule_queue,
        &reschedule_queue
    );
    mu_assert_pointers_equal(
        "scheduler.timer_list.head",
        scheduler.timer_list.tail,
        scheduler.timer_list.head
    );
    mu_assert_int_zero("scheduler.timer_list.count", scheduler.timer_list.count);
    mu_assert_int_zero("scheduler.timer", scheduler.timer);

    return NULL;
}

static void *nop(closure_t *closure){ return NULL; }
static char *should_schedule_for_later_execution(){
    DECLARE_SCHEDULER();

    sch_run_later(&scheduler, 1000, closure_create(&nop, NULL, NULL));
    mu_assert_ints_equal(
        "scheduler.timer_list.count",
        1,
        scheduler.timer_list.count
    );

    event_t *event;
    event = (event_t *)scheduler.timer_list.tail->value;
    mu_assert_ints_equal(
        "scheduler.timer_list.tail->value->timer.timeout",
        1000,
        event->timer.timeout
    );
    mu_assert_pointers_equal(
        "scheduler.timer_list.tail->value->closure.function",
        &nop,
        event->closure.function
    );

    sch_run_later(&scheduler, 500, closure_create(&nop, NULL, NULL));
    mu_assert_ints_equal(
        "scheduler.timer_list.count",
        2,
        scheduler.timer_list.count
    );

    event = (event_t *)scheduler.timer_list.tail->value;
    mu_assert_ints_equal(
        "scheduler.timer_list.tail->value->timer.timeout",
        500,
        event->timer.timeout
    );

    return NULL;
}

static char *should_schedule_intervals(){
    DECLARE_SCHEDULER();
    closure_t closure = closure_create(&nop, NULL, NULL);

    {
        sch_run_at_intervals(&scheduler, 1000, true, closure);
        llist_node_t *node = (llist_node_t *)scheduler.timer_list.tail;
        event_t *event = (event_t *)node->value;
        mu_assert_ints_equal(
            "scheduler.timer_list.count",
            1,
            scheduler.timer_list.count
        );
        mu_assert_ints_equal(
            "scheduler.timer_list.tail->timer.timeout",
            1000,
            event->timer.timeout
        );
        mu_assert(
            "scheduler.timer_list.tail->repeating must had been set",
            event->repeating
        );

    }
    {
        sch_run_at_intervals(&scheduler, 500, false, closure);
        llist_node_t *node = (llist_node_t *)scheduler.timer_list.tail;
        event_t *event = (event_t *)node->value;
        mu_assert_ints_equal(
            "scheduler.timer_list.count",
            2,
            scheduler.timer_list.count
        );
        mu_assert_ints_equal(
            "scheduler.timer_list.tail->timer.timeout",
            1000,
            event->timer.timeout
        );
    }
    {
        sch_run_at_intervals(&scheduler, 200, false, closure);
        llist_node_t *node = (llist_node_t *)scheduler.timer_list.tail;
        event_t *event = (event_t *)node->next->value;
        mu_assert_ints_equal(
            "scheduler.timer_list.count",
            3,
            scheduler.timer_list.count
        );
        mu_assert_ints_equal(
            "scheduler.timer_list.tail->next->timer.timeout",
            200,
            event->timer.timeout
        );
    }


    return NULL;
}

static void fast_forward(scheduler_t *scheduler, uint32_t *timer, uint32_t amount){
    *timer += amount;
    sch_update_timer(scheduler, *timer);
}


static void operate(scheduler_t *scheduler, evloop_t *loop){
    sch_manage_timers(scheduler);
    evloop_run(loop);
}

static void *signal_execution(closure_t *closure){
    bool *executed = (bool *)closure->context;
    *executed = true;

    return NULL;
}
static char *should_operate(){
    DECLARE_SCHEDULER();

    uint32_t timer = 0;
    evloop_t loop;
    evloop_init(&loop, &pools, &event_queue, &reschedule_queue);

    fast_forward(&scheduler, &timer, 1);
    mu_assert_ints_equal("scheduler.timer", 1, scheduler.timer);

    {
        bool success1 = false, success2 =false;

        sch_run_later(
            &scheduler,
            3,
            closure_create(&signal_execution, (void *)&success1, NULL)
        );
        sch_run_at_intervals(
            &scheduler,
            5,
            true,
            closure_create(&signal_execution, (void *)&success2, NULL)
        );
        mu_assert_not("success1 must had been unset", success1);
        mu_assert_not("success2 must had been unset", success2);

        operate(&scheduler, &loop);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert("success2 must had been set", success2);
        success2 = false;

        fast_forward(&scheduler, &timer, 3);

        operate(&scheduler, &loop);
        mu_assert("success1 must had been set", success1);
        mu_assert_not("success2 must had been unset", success2);
        success1 = false;

        fast_forward(&scheduler, &timer, 2);

        operate(&scheduler, &loop);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert("success2 must had been set", success2);
        success2 = false;

        fast_forward(&scheduler, &timer, 1);
        operate(&scheduler, &loop);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert_not("success2 must had been unset", success2);

        fast_forward(&scheduler, &timer, 4);

        operate(&scheduler, &loop);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert("success2 must had been set", success2);
    }

    return NULL;
}

char *sch_run_tests(){
    mu_run_test("should correctly initialise an scheduler", should_init_scheduler);
    mu_run_test(
        "should correctly schedule events for later execution",
        should_schedule_for_later_execution
    );
    mu_run_test(
        "should correctly schedule interval timers in both immediate and delayed modes",
        should_schedule_intervals
    );
    mu_run_test(
        "should correctly process events as they are input and run them when managing",
        should_operate
    );
    return NULL;
}
