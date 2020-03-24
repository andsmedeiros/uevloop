#include "pipeline.h"

void uel_pipeline_init(uel_pipeline_t *pipeline, uel_closure_t *closures, size_t count){
    pipeline->closures = closures;
    pipeline->count = count;
}

void *uel_pipeline_apply(uel_pipeline_t *pipeline, void *params){
    void *value = params;
    for(size_t i = 0; i < pipeline->count; i++){
        value = uel_closure_invoke(&pipeline->closures[i], value);
    }
    return value;
}
