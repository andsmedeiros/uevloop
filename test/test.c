#include <stdio.h>
#include "minunit.h"
#include "test/utils/circular-queue.h"
#include "test/utils/closure.h"
#include "test/utils/linked-list.h"


unsigned int tests_run = 0, groups_run = 0;

static char *run_all_tests(){
    mu_run_test_group("cqueue", cqueue_run_tests);
    mu_run_test_group("closure", closure_run_tests);
    mu_run_test_group("llist", llist_run_tests);
    return 0;
}

int main(int argc, char *argv[]){
    char * result = run_all_tests();
    if (result != 0) {
        printf("\nTEST FAILED!\n\n%s\n\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Groups run: %d\nTests run: %d\n", groups_run, tests_run);
    return result != 0;
}
