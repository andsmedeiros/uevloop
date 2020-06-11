#include "uevloop/utils/closure.h"

/// \cond
#include <stdlib.h>
/// \endcond

static void *nop(void *context, void *params) { return NULL; }

uel_closure_t uel_closure_create(uel_closure_function_t function, void *context) {
    uel_closure_t closure = { function, context };
    return closure;
}

void *uel_closure_invoke(uel_closure_t *closure, void *params) {
    return closure->function(closure->context, params);
}

uel_closure_t uel_nop() {
    return uel_closure_create(nop, NULL);
}
