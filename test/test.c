#include <stdlib.h>
#include <stdio.h>
#include "minunit.h"
#include "test/utils/circular-queue.h"
#include "test/utils/closure.h"
#include "test/utils/linked-list.h"
#include "test/utils/object-pool.h"
#include "test/system/event.h"
#include "test/system/pools.h"
#include "test/system/scheduler.h"


unsigned int tests_run = 0, groups_run = 0;

static char *run_all_tests(){
    mu_run_test_group("cqueue", cqueue_run_tests);
    mu_run_test_group("closure", closure_run_tests);
    mu_run_test_group("llist", llist_run_tests);
    mu_run_test_group("objpool", objpool_run_tests);
    mu_run_test_group("event", event_run_tests);
    mu_run_test_group("pools", pools_run_tests);
    mu_run_test_group("scheduler", sch_run_tests);

    return NULL;
}

int main(int argc, char *argv[]){
    char * result = run_all_tests();
    if (result != NULL) {
        printf("\nTEST FAILED!\n\n%s\n\n", result);
    } else {
        printf("\nALL TESTS PASSED!\n");
    }
    printf("Groups run: %d\nTests run: %d\n", groups_run, tests_run);
    return result != NULL;
}
