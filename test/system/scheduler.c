#include "scheduler.h"

#include <stdlib.h>

#include "utils/closure.h"
#include "system/pools.h"
#include "system/scheduler.h"
#include "../minunit.h"

closure_t update_timer;
static void set_timer_tick_handler(closure_t handler){
    update_timer = handler;
}

static void generate_timer_ticks(size_t ticks){
    for (size_t i = 0; i < ticks; i++) {
        closure_invoke(&update_timer, NULL);
    }
}

static char *should_init_scheduler(){
    pools_t pools;
    pools_init(&pools);

    scheduler_t scheduler;
    sch_init(&scheduler, &pools, &set_timer_tick_handler);

    mu_assert_pointers_equal(
        "scheduler.event_queue.buffer",
        &scheduler.event_queue_buffer,
        scheduler.event_queue.buffer
    );
    mu_assert_int_zero("scheduler.event_queue.count", scheduler.event_queue.count);

    mu_assert_pointers_equal(
        "scheduler.timer_list.head",
        scheduler.timer_list.tail,
        scheduler.timer_list.head
    );
    mu_assert_int_zero("scheduler.timer_list.count", scheduler.timer_list.count);

    mu_assert_int_zero("scheduler.timer", scheduler.timer);

    objpool_t *event_pool = pools_get(&pools, EVENT_POOL);
    mu_assert_pointers_equal(
        "scheduler.event_pool",
        event_pool,
        scheduler.event_pool
    );

    objpool_t *llist_node_pool = pools_get(&pools, LLIST_NODE_POOL);
    mu_assert_pointers_equal(
        "scheduler.llist_node_pool",
        llist_node_pool,
        scheduler.llist_node_pool
    );
    return NULL;
}

static void nop(closure_t *closure){}
static char *should_enqueue_event(){
    pools_t pools;
    pools_init(&pools);

    scheduler_t scheduler;
    sch_init(&scheduler, &pools, &set_timer_tick_handler);

    for (size_t i = 0; i < 3; i++) {
        sch_enqueue(&scheduler, closure_create(&nop, NULL, NULL));
        mu_assert_ints_equal(
            "scheduler.event_queue.count",
            i + 1,
            scheduler.event_queue.count
        );
    }

    return NULL;
}

static char *should_schedule_for_later_execution(){
    pools_t pools;
    pools_init(&pools);

    scheduler_t scheduler;
    sch_init(&scheduler, &pools, &set_timer_tick_handler);

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
    pools_t pools;
    pools_init(&pools);

    scheduler_t scheduler;
    sch_init(&scheduler, &pools, &set_timer_tick_handler);
    closure_t closure = closure_create(&nop, NULL, NULL);

    {
        sch_run_at_intervals(&scheduler, 1000, false, closure);
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
            "scheduler.timer_list.tail->timer.repeating must had been set",
            event->timer.repeating
        );

    }
    {
        sch_run_at_intervals(&scheduler, 500, true, closure);
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

        mu_assert_ints_equal(
            "scheduler.event_queue.count",
            1,
            scheduler.event_queue.count
        );
        event = cqueue_peek_tail(&scheduler.event_queue);
        mu_assert_ints_equal(
            "scheduler.event_queue.peek_tail()->timer.timeout",
            500, event->timer.timeout
        );
    }

    return NULL;
}

static void mark_execution(closure_t *closure){
    bool *executed = (bool *)closure->context;
    *executed = true;
}

static char *should_tick(){
    pools_t pools;
    pools_init(&pools);

    scheduler_t scheduler;
    sch_init(&scheduler, &pools, &set_timer_tick_handler);

    generate_timer_ticks(1);
    mu_assert_ints_equal("scheduler.timer", 1, scheduler.timer);

    {
        bool success = false;
        sch_enqueue(&scheduler, closure_create(&mark_execution, (void *)&success, NULL));
        mu_assert_not("success must had been unset", success);
        sch_tick(&scheduler);
        mu_assert("success must had been set", success);
    }
    {
        bool success1 = false, success2 =false;
        sch_enqueue(&scheduler, closure_create(&mark_execution, (void *)&success1, NULL));
        sch_enqueue(&scheduler, closure_create(&mark_execution, (void *)&success2, NULL));
        mu_assert_not("success1 must had been unset", success1);
        mu_assert_not("success2 must had been unset", success2);
        sch_tick(&scheduler);
        mu_assert("success1 must had been set", success1);
        mu_assert("success2 must had been set", success2);
    }
    {
        bool success1 = false, success2 =false;

        sch_run_later(
            &scheduler,
            3,
            closure_create(&mark_execution, (void *)&success1, NULL)
        );
        sch_run_at_intervals(
            &scheduler,
            5,
            true,
            closure_create(&mark_execution, (void *)&success2, NULL)
        );
        mu_assert_not("success1 must had been unset", success1);
        mu_assert_not("success2 must had been unset", success2);

        sch_tick(&scheduler);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert("success2 must had been set", success2);
        success2 = false;

        generate_timer_ticks(3);

        sch_tick(&scheduler);
        mu_assert("success1 must had been set", success1);
        mu_assert_not("success2 must had been unset", success2);
        success1 = false;

        generate_timer_ticks(2);

        sch_tick(&scheduler);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert("success2 must had been set", success2);
        success2 = false;

        generate_timer_ticks(1);
        sch_tick(&scheduler);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert_not("success2 must had been unset", success2);

        generate_timer_ticks(4);

        sch_tick(&scheduler);
        mu_assert_not("success1 must had been unset", success1);
        mu_assert("success2 must had been set", success2);
    }

    return NULL;
}

char *sch_run_tests(){
    mu_run_test("should correctly initialise an scheduler", should_init_scheduler);
    mu_run_test(
        "should correctly enqueue events on the scheduler's queue",
        should_enqueue_event
    );
    mu_run_test(
        "should correctly schedule events for later execution",
        should_schedule_for_later_execution
    );
    mu_run_test(
        "should correctly schedule interval timers in both immediate and delayed modes",
        should_schedule_intervals
    );
    mu_run_test(
        "should correctly tick and process events as they are input",
        should_tick
    );
    return NULL;
}
