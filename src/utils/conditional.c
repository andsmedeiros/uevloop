#include "conditional.h"

/// \cond
#include <stdlib.h>
/// \endcond

void uel_conditional_init(uel_conditional_t *conditional, uel_closure_t test,
                                uel_closure_t if_true, uel_closure_t if_false){
    conditional->test = test;
    conditional->if_true = if_true;
    conditional->if_false = if_false;
}

void *uel_conditional_apply(uel_conditional_t *conditional, void *params){
    if(uel_closure_invoke(&conditional->test, params)){
        return uel_closure_invoke(&conditional->if_true, params);
    }else{
        return uel_closure_invoke(&conditional->if_false, params);
    }
}
