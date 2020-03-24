#include "pipeline.h"

#include <stdlib.h>
#include <stdbool.h>

#include "../uelt.h"
#include "utils/pipeline.h"

void *nop1(uel_closure_t *closure){ return NULL; }
void *nop2(uel_closure_t *closure){ return NULL; }

static char *should_initialise_pipeline(){
    UEL_PIPELINE_DECLARE(test,
        uel_closure_create(nop1, NULL, NULL),
        uel_closure_create(nop2, NULL, NULL)
    );

    uelt_assert_pointers_equal(
        "test_pipeline.closures",
        &test_pipeline_closures,
        test_pipeline.closures
    );
    uelt_assert_ints_equal("test_pipeline.count", 2, test_pipeline.count);

    return NULL;
}

static void *mark_flag(uel_closure_t *closure){
    bool *flag = (bool *)closure->context;
    bool value = (bool)closure->params;
    *flag = value;

    return (void *)value;
}

static void *increment_and_store(uel_closure_t *closure){
    uintptr_t *slot = (uintptr_t *)closure->context;
    uintptr_t value = (uintptr_t)closure->params;

    *slot = value + 1;

    return (void *)*slot;
}
static char *should_operate_pipeline(){
    bool flags[] = {false, false, false};
    UEL_PIPELINE_DECLARE(flags,
        uel_closure_create(mark_flag, (void *)&flags[0], NULL),
        uel_closure_create(mark_flag, (void *)&flags[1], NULL),
        uel_closure_create(mark_flag, (void *)&flags[2], NULL)
    );

    uel_pipeline_apply(&flags_pipeline, (void *)true);

    uelt_assert("flags[0]", flags[0]);
    uelt_assert("flags[1]", flags[1]);
    uelt_assert("flags[2]", flags[2]);

    uel_pipeline_apply(&flags_pipeline, (void *)false);

    uelt_assert_not("flags[0]", flags[0]);
    uelt_assert_not("flags[1]", flags[1]);
    uelt_assert_not("flags[2]", flags[2]);

    uintptr_t numbers[] = {0, 0, 0};
    UEL_PIPELINE_DECLARE(nums,
        uel_closure_create(increment_and_store, (void *)&numbers[0], NULL),
        uel_closure_create(increment_and_store, (void *)&numbers[1], NULL),
        uel_closure_create(increment_and_store, (void *)&numbers[2], NULL),
    );

    uel_pipeline_apply(&nums_pipeline, (void *)5);

    uelt_assert_ints_equal("numbers[0]", 6, numbers[0]);
    uelt_assert_ints_equal("numbers[1]", 7, numbers[1]);
    uelt_assert_ints_equal("numbers[2]", 8, numbers[2]);

    return NULL;
}

char *uel_pipeline_run_tests(){
    uelt_run_test(
        "should correctly initialise a pipeline object",
        should_initialise_pipeline
    );
    uelt_run_test("should correctly operate a pipeline", should_operate_pipeline);

    return NULL;
}
