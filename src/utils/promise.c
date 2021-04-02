#include "uevloop/utils/promise.h"
#include "uevloop/portability/critical-section.h"

#include <stddef.h>

static void *rejecter(void *context, void *params) {
    uel_promise_t *promise = (uel_promise_t *)context;
    uel_promise_reject(promise, params);

    return NULL;
}

static void *resolver(void *context, void *params) {
    uel_promise_t *promise = (uel_promise_t *)context;
    uel_promise_resolve(promise, params);

    return NULL;
}

static void *destroyer(void *context, void *params) {
    uel_promise_t *promise = (uel_promise_t *)context;

    UEL_CRITICAL_ENTER;
    uel_objpool_release(promise->source->promise_pool, (void *)promise);
    UEL_CRITICAL_EXIT;

    return NULL;
}

static inline void push_segment(uel_promise_t *promise, uel_promise_segment_t *segment) {
    if (promise->first_segment == NULL) {
        promise->first_segment = segment;
    } else {
        promise->last_segment->next = segment;
    }
    promise->last_segment = segment;
}

static inline void await_promise(uel_promise_t *promise, uel_promise_t *other) {
    promise->state = UEL_PROMISE_PENDING;

    uel_promise_segment_t *segment;
    UEL_CRITICAL_ENTER;
    segment = (uel_promise_segment_t *)uel_objpool_acquire(promise->source->segment_pool);
    UEL_CRITICAL_EXIT;

    segment->next = promise->first_segment;
    segment->reject = uel_promise_destroyer(promise);
    segment->resolve = uel_promise_destroyer(promise);
    promise->first_segment = segment;
    if(promise->last_segment == NULL) {
        promise->last_segment = segment;
    }

    uel_promise_after(
        other,
        uel_promise_resolver(promise),
        uel_promise_rejecter(promise)
    );
}

static inline void process_segment(uel_promise_t *promise) {
    uel_promise_segment_t *segment;
    UEL_CRITICAL_ENTER;
    segment = promise->first_segment;
    promise->first_segment = segment->next;
    if(!promise->first_segment) promise->last_segment = NULL;
    UEL_CRITICAL_EXIT;

    uel_promise_t *other = NULL;
    switch (promise->state) {
        case UEL_PROMISE_RESOLVED:
            other = (uel_promise_t *)uel_closure_invoke(&segment->resolve, (void *)promise);
            break;
        case UEL_PROMISE_REJECTED:
            other = (uel_promise_t *)uel_closure_invoke(&segment->reject, (void *)promise);
            break;
        default: break;
    }
    if (other) {
        await_promise(promise, other);
    }

    UEL_CRITICAL_ENTER;
    uel_objpool_release(promise->source->segment_pool, (void *)segment);
    UEL_CRITICAL_EXIT;
}

static inline void flush_segments(uel_promise_t *promise) {
    while (promise->state != UEL_PROMISE_PENDING && promise->first_segment) {
        process_segment(promise);
    }
}

uel_promise_store_t uel_promise_store_create(
    uel_objpool_t *promise_pool,
    uel_objpool_t *segment_pool
) {
    uel_promise_store_t store = {
        .promise_pool = promise_pool,
        .segment_pool = segment_pool
    };
    return store;
}
uel_promise_t *uel_promise_create(uel_promise_store_t *store, uel_closure_t closure) {
    uel_promise_t *promise;
    UEL_CRITICAL_ENTER;
    promise = (uel_promise_t *)uel_objpool_acquire(store->promise_pool);
    UEL_CRITICAL_EXIT;

    promise->source = store;
    promise->state = UEL_PROMISE_PENDING;
    promise->value = NULL;
    promise->first_segment = NULL;
    promise->last_segment = NULL;
    uel_closure_invoke(&closure, promise);

    return promise;
}

void uel_promise_destroy(uel_promise_t *promise) {
    uel_promise_segment_t *segment;
    for(segment = promise->first_segment; segment; segment = segment->next) {
        UEL_CRITICAL_ENTER;
        uel_objpool_release(promise->source->segment_pool, (void *)segment);
        UEL_CRITICAL_EXIT;
    }
    UEL_CRITICAL_ENTER;
    uel_objpool_release(promise->source->promise_pool, (void *)promise);
    UEL_CRITICAL_EXIT;
}

void uel_promise_then(uel_promise_t *promise, uel_closure_t resolve) {
    uel_promise_after(promise, resolve, uel_nop());
}

void uel_promise_catch(uel_promise_t *promise, uel_closure_t reject) {
    uel_promise_after(promise, uel_nop(), reject);
}

void uel_promise_always(uel_promise_t *promise, uel_closure_t always) {
    uel_promise_after(promise, always, always);
}

void uel_promise_after(
    uel_promise_t *promise,
    uel_closure_t resolve,
    uel_closure_t reject
) {
    uel_promise_segment_t *segment;
    UEL_CRITICAL_ENTER;
    segment = (uel_promise_segment_t *)uel_objpool_acquire(promise->source->segment_pool);
    UEL_CRITICAL_EXIT;

    segment ->next = NULL;
    segment->resolve = resolve;
    segment->reject = reject;
    push_segment(promise, segment);
    flush_segments(promise);
}

void uel_promise_resolve(uel_promise_t *promise, void *value) {
    promise->value = value;
    promise->state = UEL_PROMISE_RESOLVED;
    flush_segments(promise);
}

void uel_promise_reject(uel_promise_t *promise, void *value) {
    promise->value = value;
    promise->state = UEL_PROMISE_REJECTED;
    flush_segments(promise);
}

void uel_promise_resettle(
    uel_promise_t *promise,
    uel_promise_state_t state,
    void *value
) {
    promise->state = state;
    promise->value = value;
}

uel_closure_t uel_promise_resolver(uel_promise_t *promise) {
    return uel_closure_create(resolver, (void *)promise);
}

uel_closure_t uel_promise_rejecter(uel_promise_t *promise) {
    return uel_closure_create(rejecter, (void *)promise);
}
uel_closure_t uel_promise_destroyer(uel_promise_t *promise) {
    return uel_closure_create(destroyer, (void *)promise);
}
