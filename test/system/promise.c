#include "promise.h"
#include "test/uelt.h"
#include "uevloop/utils/object-pool.h"
#include "uevloop/utils/closure.h"
#include "uevloop/system/promise.h"

#include <stddef.h>

#define DECLARE_STORE                                               \
    UEL_DECLARE_OBJPOOL_BUFFERS(uel_promise_t, 4, promise);         \
    uel_objpool_t promise_pool;                                     \
    uel_objpool_init(                                               \
        &promise_pool,                                              \
        4,                                                          \
        sizeof(uel_promise_t),                                      \
        UEL_OBJPOOL_BUFFERS(promise)                                \
    );                                                              \
    UEL_DECLARE_OBJPOOL_BUFFERS(uel_promise_segment_t, 6, segment); \
    uel_objpool_t segment_pool;                                     \
    uel_objpool_init(                                               \
        &segment_pool,                                              \
        6,                                                          \
        sizeof(uel_promise_segment_t),                              \
        UEL_OBJPOOL_BUFFERS(segment)                                \
    );                                                              \
    uel_promise_store_t store =                                     \
        uel_promise_store_create(&promise_pool, &segment_pool);

static char *should_create_promise_store() {
    DECLARE_STORE;

    uelt_assert_pointers_equal("store.promise_pool", &promise_pool, store.promise_pool);
    uelt_assert_pointers_equal("store.segment_pool", &segment_pool, store.segment_pool);

    return NULL;
}

static char *should_create_and_destroy_promise() {
    DECLARE_STORE;

    size_t old_count = store.promise_pool->queue.count;
    uel_promise_t *promise = uel_promise_create(&store, uel_nop());
    size_t new_count = store.promise_pool->queue.count;

    uelt_assert_ints_equal("promise count", old_count - 1, new_count);
    uelt_assert_pointers_equal("promise->source", &store, promise->source);
    uelt_assert_ints_equal("promise->state", UEL_PROMISE_PENDING, promise->state);
    uelt_assert_pointer_null("promise->value", promise->value);
    uelt_assert_pointer_null("promise->first_segment", promise->first_segment);
    uelt_assert_pointer_null("promise->last_segment", promise->last_segment);

    old_count = store.segment_pool->queue.count;
    uel_promise_then(promise, uel_nop());
    new_count = store.segment_pool->queue.count;
    uelt_assert_ints_equal("segment count", old_count - 1, new_count);

    uel_promise_destroy(promise);
    uelt_assert_ints_equal(
        "promise count after destroy",
        store.promise_pool->queue.size,
        store.promise_pool->queue.count
    );
    uelt_assert_ints_equal(
        "segment count after destroy",
        store.segment_pool->queue.size,
        store.segment_pool->queue.count
    );

    return NULL;
}

static void *nop1(uel_closure_t *closure) { return NULL; }
static void *nop2(uel_closure_t *closure) { return NULL; }
static void *nop3(uel_closure_t *closure) { return NULL; }
static void *nop4(uel_closure_t *closure) { return NULL; }
static void *nop5(uel_closure_t *closure) { return NULL; }
static char *should_enqueue_segments() {
    DECLARE_STORE;

    uintptr_t a, b, c, d, e;
    uel_promise_t *promise = uel_promise_create(&store, uel_nop());

    uel_promise_then(promise, uel_closure_create(nop1, (void *)&a, NULL));
    uelt_assert_pointer_not_null("promise->first_segment", promise->first_segment);
    uelt_assert_pointer_not_null("promise->last_segment", promise->last_segment);
    uelt_assert_pointers_equal(
        "promise->first_segment",
        promise->last_segment,
        promise->first_segment
    );
    uelt_assert_pointers_equal(
        "promise->first_segment->resolve.function",
        &nop1,
        promise->first_segment->resolve.function
    );
    uelt_assert_pointers_equal(
        "promise->first_segment->resolve.context",
        &a,
        promise->first_segment->resolve.context
    );

    uel_promise_after(
        promise,
        uel_closure_create(nop2, (void *)&b, NULL),
        uel_closure_create(nop3, (void *)&c, NULL)
    );
    uelt_assert_pointer_not_null("promise->first_segment", promise->first_segment);
    uelt_assert_pointer_not_null("promise->last_segment", promise->last_segment);
    uelt_assert_pointers_not_equal(
        "promise->first_segment",
        promise->last_segment,
        promise->first_segment
    );
    uelt_assert_pointers_equal(
        "promise->first_segment->next",
        promise->last_segment,
        promise->first_segment->next
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->resolve.function",
        &nop2,
        promise->last_segment->resolve.function
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->resolve.context",
        &b,
        promise->last_segment->resolve.context
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->reject.function",
        &nop3,
        promise->last_segment->reject.function
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->reject.context",
        &c,
        promise->last_segment->reject.context
    );

    uel_promise_catch(promise, uel_closure_create(nop4, (void *)&d, NULL));
    uelt_assert_pointer_not_null("promise->first_segment", promise->first_segment);
    uelt_assert_pointer_not_null("promise->last_segment", promise->last_segment);
    uelt_assert_pointers_equal(
        "promise->first_segment->next->next",
        promise->last_segment,
        promise->first_segment->next->next
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->reject.function",
        &nop4,
        promise->last_segment->reject.function
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->reject.context",
        &d,
        promise->last_segment->reject.context
    );

    uel_promise_always(promise, uel_closure_create(nop5, (void *)&e, NULL));
    uelt_assert_pointer_not_null("promise->first_segment", promise->first_segment);
    uelt_assert_pointer_not_null("promise->last_segment", promise->last_segment);
    uelt_assert_pointers_equal(
        "promise->first_segment->next->next->next",
        promise->last_segment,
        promise->first_segment->next->next->next
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->resolve.function",
        &nop5,
        promise->last_segment->resolve.function
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->resolve.context",
        &e,
        promise->last_segment->resolve.context
    );
    uelt_assert_pointers_equal(
        "promise->last_segment->reject.function",
        &nop5,
        promise->last_segment->reject.function
    );
    uelt_assert_pointers_equal(
        "promise->first_segment->next->next->next->reject.context",
        &e,
        promise->last_segment->reject.context
    );
    return NULL;
}

static void *resolves(uel_closure_t *closure) {
    uel_promise_t *promise = (uel_promise_t *)closure->params;
    uel_promise_resolve(promise, closure->context);
    return NULL;
}
static void *rejects(uel_closure_t *closure) {
    uel_promise_t *promise = (uel_promise_t *)closure->params;
    uel_promise_reject(promise, closure->context);
    return NULL;
}
static void *store_char(uel_closure_t *closure) {
    unsigned char *var = (unsigned char *)closure->context;
    uel_promise_t *promise = (uel_promise_t *)closure->params;
    unsigned char value = (unsigned char)(uintptr_t)promise->value;
    *var = value;

    return NULL;
}
static char *should_operate_immediately_resolved_promises() {
    DECLARE_STORE;

    uel_promise_t *promise = uel_promise_create(
        &store,
        uel_closure_create(resolves, (void *)((uintptr_t)'x'), NULL)
    );

    uelt_assert_ints_equal("promise->state", UEL_PROMISE_RESOLVED, promise->state);
    uelt_assert_equals("promise->value", 'x', (uintptr_t)promise->value, "%c");

    unsigned char a = '0', b = '0';
    uel_promise_after(
        promise,
        uel_closure_create(store_char, (void *)&a, NULL),
        uel_closure_create(store_char, (void *)&b, NULL)
    );
    uelt_assert_equals("a", 'x', a, "%c");
    uelt_assert_equals("b", '0', b, "%c");

    unsigned char c = '0', d = '0';
    uel_promise_catch(promise, uel_closure_create(store_char, (void *)&c, NULL));
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    uel_promise_then(promise, uel_closure_create(store_char, (void *)&d, NULL));
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", 'x', d, "%c");

    return NULL;
}

static char *should_operate_immediately_rejected_promises() {
    DECLARE_STORE;

    uel_promise_t *promise = uel_promise_create(
        &store,
        uel_closure_create(rejects, (void *)((uintptr_t)'x'), NULL)
    );

    uelt_assert_ints_equal("promise->state", UEL_PROMISE_REJECTED, promise->state);
    uelt_assert_equals("promise->value", 'x', (uintptr_t)promise->value, "%c");

    unsigned char a = '0', b = '0';
    uel_promise_after(
        promise,
        uel_closure_create(store_char, (void *)&a, NULL),
        uel_closure_create(store_char, (void *)&b, NULL)
    );
    uelt_assert_equals("a", '0', a, "%c");
    uelt_assert_equals("b", 'x', b, "%c");

    unsigned char c = '0', d = '0';
    uel_promise_catch(promise, uel_closure_create(store_char, (void *)&c, NULL));
    uelt_assert_equals("c", 'x', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    uel_promise_then(promise, uel_closure_create(store_char, (void *)&d, NULL));
    uelt_assert_equals("c", 'x', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    return NULL;
}

static char *should_operate_late_resolved_promises() {
    DECLARE_STORE;

    uel_promise_t *promise = uel_promise_create(&store, uel_nop());

    uelt_assert_ints_equal("promise->state", UEL_PROMISE_PENDING, promise->state);

    unsigned char a = '0', b = '0';
    uel_promise_after(
        promise,
        uel_closure_create(store_char, (void *)&a, NULL),
        uel_closure_create(store_char, (void *)&b, NULL)
    );
    uelt_assert_equals("a", '0', a, "%c");
    uelt_assert_equals("b", '0', b, "%c");

    unsigned char c = '0', d = '0';
    uel_promise_catch(promise, uel_closure_create(store_char, (void *)&c, NULL));
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    uel_promise_then(promise, uel_closure_create(store_char, (void *)&d, NULL));
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    uel_promise_resolve(promise, (void *)((uintptr_t)'x'));

    uelt_assert_equals("promise->value", 'x', (uintptr_t)promise->value, "%c");
    uelt_assert_equals("a", 'x', a, "%c");
    uelt_assert_equals("b", '0', b, "%c");
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", 'x', d, "%c");
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", 'x', d, "%c");

    return NULL;
}

static char *should_operate_late_rejected_promises() {
    DECLARE_STORE;

    uel_promise_t *promise = uel_promise_create(&store, uel_nop());

    uelt_assert_ints_equal("promise->state", UEL_PROMISE_PENDING, promise->state);

    unsigned char a = '0', b = '0';
    uel_promise_after(
        promise,
        uel_closure_create(store_char, (void *)&a, NULL),
        uel_closure_create(store_char, (void *)&b, NULL)
    );
    uelt_assert_equals("a", '0', a, "%c");
    uelt_assert_equals("b", '0', b, "%c");

    unsigned char c = '0', d = '0';
    uel_promise_catch(promise, uel_closure_create(store_char, (void *)&c, NULL));
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    uel_promise_then(promise, uel_closure_create(store_char, (void *)&d, NULL));
    uelt_assert_equals("c", '0', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    uel_promise_reject(promise, (void *)((uintptr_t)'x'));

    uelt_assert_equals("promise->value", 'x', (uintptr_t)promise->value, "%c");
    uelt_assert_equals("a", '0', a, "%c");
    uelt_assert_equals("b", 'x', b, "%c");
    uelt_assert_equals("c", 'x', c, "%c");
    uelt_assert_equals("d", '0', d, "%c");

    return NULL;
}

static void *resettle_resolved(uel_closure_t *closure) {
    uel_promise_t *promise = (uel_promise_t *)closure->params;
    uel_promise_resettle(promise, UEL_PROMISE_RESOLVED, closure->context);
    return NULL;
}
static void *resettle_rejected(uel_closure_t *closure) {
    uel_promise_t *promise = (uel_promise_t *)closure->params;
    uel_promise_resettle(promise, UEL_PROMISE_REJECTED, closure->context);
    return NULL;
}
static void *resettle_pending(uel_closure_t *closure) {
    uel_promise_t *promise = (uel_promise_t *)closure->params;
    uel_promise_resettle(promise, UEL_PROMISE_PENDING, closure->context);
    return NULL;
}
char *should_resettle() {
    DECLARE_STORE;

    uel_promise_t *promise = uel_promise_create(&store, uel_nop());
    char steps[8] = "0000000";
    uel_promise_always(promise, uel_closure_create(store_char, (void *)&steps[0], NULL));
    uel_promise_always(promise, uel_closure_create(store_char, (void *)&steps[1], NULL));
    uel_promise_after(
        promise,
        uel_closure_create(resettle_rejected, (void *)((uintptr_t)'e'), NULL),
        uel_closure_create(store_char, (void *)&steps[2], NULL)
    );
    uel_promise_always(promise, uel_closure_create(store_char, (void *)&steps[3], NULL));
    uel_promise_after(
        promise,
        uel_closure_create(store_char, (void *)&steps[4], NULL),
        uel_closure_create(resettle_resolved, (void *)((uintptr_t)'s'), NULL)
    );
    uel_promise_always(promise, uel_closure_create(store_char, (void *)&steps[5], NULL));
    uel_promise_always(promise, uel_closure_create(resettle_pending, NULL, NULL));
    uel_promise_always(promise, uel_closure_create(store_char, (void *)&steps[6], NULL));

    uelt_assert_strs_equal("steps before resolution", "0000000", steps);

    uel_promise_resolve(promise, (void *)((uintptr_t)'s'));
    uelt_assert_strs_equal("steps after resolution", "ss0e0s0", steps);
    uelt_assert_ints_equal("promise->state", UEL_PROMISE_PENDING, promise->state);
    uelt_assert_pointer_null("promise->value", promise->value);
    uelt_assert_pointer_not_null("promise->first_segment", promise->first_segment);
    uelt_assert_pointers_equal(
        "promise->first_segment",
        promise->last_segment,
        promise->first_segment
    );
    uelt_assert_pointer_null(
        "promise->first_segment->next",
        promise->first_segment->next
    );

    return NULL;
}

static void *deref_context(uel_closure_t *closure) {
    return closure->context;
}
static void *mark_execution(uel_closure_t *closure) {
    bool *flag = (bool *)closure->context;
    *flag = true;
    return NULL;
}
char *should_handle_subpromises() {
    DECLARE_STORE;

    uel_promise_t *p1 = uel_promise_create(&store, uel_nop());
    uel_promise_t *p2 = uel_promise_create(&store, uel_nop());
    uel_promise_t *p3 = uel_promise_create(&store, uel_nop());

    bool b1 = false, b2 = false, b3 = false;

    uel_promise_always(p1, uel_closure_create(mark_execution, (void *)&b1, NULL));
    uel_promise_always(p1, uel_closure_create(deref_context, (void *)p2, NULL));

    uel_promise_always(p2, uel_closure_create(mark_execution, (void *)&b2, NULL));
    uel_promise_always(p2, uel_closure_create(deref_context, (void *)p3, NULL));

    uel_promise_always(p3, uel_closure_create(mark_execution, (void *)&b3, NULL));

    uelt_assert_ints_equal("p1->state", UEL_PROMISE_PENDING, p1->state);
    uelt_assert_not("b1", b1);
    uelt_assert_ints_equal("p2->state", UEL_PROMISE_PENDING, p2->state);
    uelt_assert_not("b2", b2);
    uelt_assert_ints_equal("p3->state", UEL_PROMISE_PENDING, p3->state);
    uelt_assert_not("b3", b3);

    uel_promise_resolve(p1, NULL);
    uelt_assert_ints_equal("p1->state", UEL_PROMISE_PENDING, p1->state);
    uelt_assert("b1", b1);
    uelt_assert_ints_equal("p2->state", UEL_PROMISE_PENDING, p2->state);
    uelt_assert_not("b2", b2);
    uelt_assert_ints_equal("p3->state", UEL_PROMISE_PENDING, p3->state);
    uelt_assert_not("b3", b3);

    uel_promise_reject(p3, NULL);
    uelt_assert_ints_equal("p1->state", UEL_PROMISE_PENDING, p1->state);
    uelt_assert("b1", b1);
    uelt_assert_ints_equal("p2->state", UEL_PROMISE_PENDING, p2->state);
    uelt_assert_not("b2", b2);
    uelt_assert_ints_equal("p3->state", UEL_PROMISE_REJECTED, p3->state);
    uelt_assert("b3", b3);

    uel_promise_resolve(p2, NULL);
    uelt_assert_ints_equal("p1->state", UEL_PROMISE_REJECTED, p1->state);
    uelt_assert("b1", b1);
    uelt_assert_ints_equal("p2->state", UEL_PROMISE_REJECTED, p2->state);
    uelt_assert("b2", b2);
    uelt_assert_ints_equal("p3->state", UEL_PROMISE_REJECTED, p3->state);
    uelt_assert("b3", b3);
    return NULL;
}

static char *should_supply_helpers() {
    DECLARE_STORE;

    uel_promise_t *p1 = uel_promise_create(&store, uel_nop());
    uel_promise_t *p2 = uel_promise_create(&store, uel_nop());
    uel_promise_t *p3 = uel_promise_create(&store, uel_nop());

    uel_closure_t resolver = uel_promise_resolver(p1);
    uel_closure_t rejecter = uel_promise_rejecter(p2);
    uel_closure_t destroyer = uel_promise_destroyer(p3);

    uel_closure_invoke(&resolver, (void *)1);
    uelt_assert_ints_equal("p1->state", UEL_PROMISE_RESOLVED, p1->state);
    uelt_assert_ints_equal("p1->value", (void *)1, p1->value);

    uel_closure_invoke(&rejecter, (void *)2);
    uelt_assert_ints_equal("p2->state", UEL_PROMISE_REJECTED, p2->state);
    uelt_assert_ints_equal("p2->value", (void *)2, p2->value);

    size_t old_count = store.promise_pool->queue.count;
    uel_closure_invoke(&destroyer, (void *)3);
    size_t new_count = store.promise_pool->queue.count;
    uelt_assert_ints_equal("promise count", old_count + 1, new_count);

    return NULL;
}

char *uel_promise_run_tests() {
    uelt_run_test(
        "should correctly create a promise store",
        should_create_promise_store
    );
    uelt_run_test(
        "should correctly create and destroy a promise",
        should_create_and_destroy_promise
    );
    uelt_run_test(
        "should correctly enqueue various segments",
        should_enqueue_segments
    );
    uelt_run_test(
        "should correctly operate immediately resolved promises",
        should_operate_immediately_resolved_promises
    );
    uelt_run_test(
        "should correctly operate immediately rejected promises",
        should_operate_immediately_rejected_promises
    );
    uelt_run_test(
        "should correctly operate late resolved promises",
        should_operate_late_resolved_promises
    );
    uelt_run_test(
        "should correctly operate late rejected promises",
        should_operate_late_rejected_promises
    );
    uelt_run_test("should correctly resettle promises", should_resettle);
    uelt_run_test("should correctly handle sub-promisses", should_handle_subpromises);
    uelt_run_test("should correctly supply helper closures", should_supply_helpers);

    return NULL;
}
