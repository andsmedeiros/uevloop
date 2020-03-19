#include <stdlib.h>
#include <stdio.h>
#include "uelt.h"
#include "test/utils/circular-queue.h"
#include "test/utils/closure.h"
#include "test/utils/linked-list.h"
#include "test/utils/object-pool.h"
#include "test/system/containers/system-pools.h"
#include "test/system/containers/system-queues.h"
#include "test/system/containers/application.h"
#include "test/system/event.h"
#include "test/system/scheduler.h"
#include "test/system/event-loop.h"
#include "test/system/signal.h"

uelt_context_t test_context = DEFAULT_TEST_CONTEXT;

static char *run_all_tests(){
    uelt_run_test_group("cqueue", uel_cqueue_run_tests);
    uelt_run_test_group("closure", uel_closure_run_tests);
    uelt_run_test_group("llist", uel_llist_run_tests);
    uelt_run_test_group("objpool", objpool_run_tests);
    uelt_run_test_group("syspools", uel_syspools_run_tests);
    uelt_run_test_group("sysqueues", uel_sysqueues_run_tests);
    uelt_run_test_group("event", event_run_tests);
    uelt_run_test_group("scheduler", sch_run_tests);
    uelt_run_test_group("evloop", uel_evloop_run_tests);
    uelt_run_test_group("signal", uel_signal_run_tests);
    uelt_run_test_group("app", uel_app_run_tests);

    return NULL;
}

int main(int argc, char *argv[]){
    test_context = (uelt_context_t)DEFAULT_TEST_CONTEXT;

    char * result = run_all_tests();
    if (result != NULL) {
        printf("\nTEST FAILED!\n\n%s\n\n", result);
    } else {
        printf("\nALL TESTS PASSED!\n");
    }
    printf(
        "Groups run: %d\nTests run: %d\nAssertions run: %d\n",
        test_context.groups_run,
        test_context.tests_run,
        test_context.assertions_run
    );
    return result != NULL;
}
