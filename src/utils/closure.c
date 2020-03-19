#include "closure.h"

/// \cond
#include <stdlib.h>
/// \endcond

uel_closure_t uel_closure_create(
    void * (* function)(uel_closure_t *),
    void *context,
    void (* destructor)(uel_closure_t *)
){
    uel_closure_t closure = { function, context, destructor, NULL, NULL };
    return closure;
}

void *uel_closure_invoke(uel_closure_t *closure, void *params){
    closure->params = params;
    closure->value = closure->function(closure);
    return closure->value;
}

void uel_closure_destroy(uel_closure_t *closure){
    if(closure->destructor != NULL) {
        closure->destructor(closure);
    }
}
