#include "uevloop/utils/module.h"

void uel_module_init(
    uel_module_t *module,
    uel_module_hook_t config,
    uel_module_hook_t launch,
    struct uel_application *app
){
    module->config = config;
    module->launch = launch;
    module->app = app;
}

void uel_module_config(uel_module_t *module){
    module->config(module);
}

void uel_module_launch(uel_module_t *module){
    module->launch(module);
}
