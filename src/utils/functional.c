#include "uevloop/utils/functional.h"

/// \cond
#include <stdbool.h>
/// \endcond

static void *uel_func_unwrap_pipeline(void *context, void *params){
    uel_pipeline_t *pipeline = (uel_pipeline_t *)context;
    return uel_pipeline_apply(pipeline, params);
}

static void *uel_func_unwrap_conditional(void *context, void *params){
    uel_conditional_t *conditional = (uel_conditional_t *)context;
    return uel_conditional_apply(conditional, params);
}

static void *uel_func_unwrap_foreach(void *context, void *params){
    uel_iterator_t *iterator = (uel_iterator_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    bool completed = uel_iterator_foreach(iterator, wrapped);
    return (void *)completed;
}

static void *uel_func_unwrap_map(void *context, void *params){
    uel_func_mapper_t *mapper = (uel_func_mapper_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    uel_iterator_map(mapper->iterator, wrapped, mapper->destination, mapper->limit);
    return (void *)mapper->destination;
}

static void *uel_func_unwrap_find(void *context, void *params){
    uel_iterator_t *iterator = (uel_iterator_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    return uel_iterator_find(iterator, wrapped);
}

static void *uel_func_unwrap_count(void *context, void *params){
    uel_iterator_t *iterator = (uel_iterator_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    return (void *)(uintptr_t)uel_iterator_count(iterator, wrapped);
}

static void *uel_func_unwrap_all(void *context, void *params){
    uel_iterator_t *iterator = (uel_iterator_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    return (void *)uel_iterator_all(iterator, wrapped);
}

static void *uel_func_unwrap_none(void *context, void *params){
    uel_iterator_t *iterator = (uel_iterator_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    return (void *)uel_iterator_none(iterator, wrapped);
}

static void *uel_func_unwrap_any(void *context, void *params){
    uel_iterator_t *iterator = (uel_iterator_t *)params;
    uel_closure_t *wrapped = (uel_closure_t *)context;
    return (void *)uel_iterator_any(iterator, wrapped);
}

void uel_func_mapper_init(uel_func_mapper_t *mapper, uel_iterator_t *iterator,
                                          void **destination, size_t limit){
    mapper->iterator = iterator;
    mapper->destination = destination;
    mapper->limit = limit;
}

uel_closure_t uel_func_pipeline(uel_pipeline_t *pipeline){
    uel_closure_t closure =
        uel_closure_create(uel_func_unwrap_pipeline, (void *)pipeline);
    return closure;
}

uel_closure_t uel_func_conditional(uel_conditional_t *conditional){
    uel_closure_t closure =
        uel_closure_create(uel_func_unwrap_conditional, (void *)conditional);
    return closure;
}

uel_closure_t uel_func_foreach(uel_closure_t *closure){
    return uel_closure_create( uel_func_unwrap_foreach, (void *)closure);
}

uel_closure_t uel_func_map(uel_closure_t *closure){
    return uel_closure_create(uel_func_unwrap_map, (void *)closure);
}

uel_closure_t uel_func_find(uel_closure_t *closure){
    return uel_closure_create(uel_func_unwrap_find, (void *)closure);
}

uel_closure_t uel_func_count(uel_closure_t *closure){
    return uel_closure_create(uel_func_unwrap_count, (void *)closure);
}

uel_closure_t uel_func_all(uel_closure_t *closure){
    return uel_closure_create(uel_func_unwrap_all, (void *)closure);
}

uel_closure_t uel_func_none(uel_closure_t *closure){
    return uel_closure_create(uel_func_unwrap_none, (void *)closure);
}

uel_closure_t uel_func_any(uel_closure_t *closure){
    return uel_closure_create(uel_func_unwrap_any, (void *)closure);
}
