#include "closure.h"
#include <stdlib.h>

closure_t closure_create(
    void (* function)(closure_t *), 
    void *context,
    void (* destructor)(closure_t *)
){
    closure_t closure = { function, context, destructor, NULL, NULL }; 
    return closure;
}

void *closure_invoke(closure_t *closure, void *params){
    closure->params = params;
    closure->function(closure);
    return closure->value;
} 

void closure_destroy(closure_t *closure){
    if(closure->destructor != NULL) {
        closure->destructor(closure);
    }
}

void closure_return(closure_t *closure, void *value){
    closure->value = value;
}