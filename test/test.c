#include <stdlib.h>
#include <stdio.h>
#include "minunit.h"
#include "test/utils/circular-queue.h"
#include "test/utils/closure.h"
#include "test/utils/linked-list.h"
#include "test/utils/object-pool.h"
#include "test/system/event.h"
#include "test/system/syspools.h"
#include "test/system/scheduler.h"
#include "test/system/event-loop.h"
#include "test/system/signal.h"
#include "test/system/application.h"

minunit_context_t test_context = DEFAULT_TEST_CONTEXT;

static char *run_all_tests(){
    mu_run_test_group("cqueue", cqueue_run_tests);
    mu_run_test_group("closure", closure_run_tests);
    mu_run_test_group("llist", llist_run_tests);
    mu_run_test_group("objpool", objpool_run_tests);
    mu_run_test_group("event", event_run_tests);
    mu_run_test_group("syspools", syspools_run_tests);
    mu_run_test_group("scheduler", sch_run_tests);
    mu_run_test_group("evloop", evloop_run_tests);
    mu_run_test_group("signal", signal_run_tests);
    mu_run_test_group("app", app_run_tests);

    return NULL;
}

int main(int argc, char *argv[]){
    test_context = (minunit_context_t)DEFAULT_TEST_CONTEXT;

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
