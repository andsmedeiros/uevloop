#include "module.h"

#include <stdlib.h>
#include <stdbool.h>
#include "utils/module.h"
#include <system/containers/application.h>
#include "test/uelt.h"

static bool config_done = false, launch_done = false;
static void config(uel_module_t *mod){ config_done = true; }
static void launch(uel_module_t *mod){ launch_done = true; }

static char *should_init_module(){
    uel_application_t app;
    uel_app_init(&app);

    uel_module_t module;
    uel_module_init(&module, config, launch, &app);
    uelt_assert_pointers_equal("module.config", config, module.config);
    uelt_assert_pointers_equal("module.launch", launch, module.launch);
    uelt_assert_pointers_equal("module.app", &app, module.app);

    return NULL;
}

static char *should_operate_module(){
    uel_application_t app;
    uel_app_init(&app);
    uel_module_t module;
    uel_module_init(&module, config, launch, &app);

    uel_module_config(&module);
    uelt_assert("config_done", config_done);

    uel_module_launch(&module);
    uelt_assert("launch_done", launch_done);

    return NULL;
}

char *uel_module_run_tests(){

    uelt_run_test("should correctly initialise a module", should_init_module);
    uelt_run_test("should correctly operate a module", should_operate_module);

    return NULL;
}
