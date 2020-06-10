#include "uevloop/system/signal.h"

/// \cond
#include <stdbool.h>
#include <stdlib.h>
/// \endcond

#include "uevloop/config.h"
#include "uevloop/portability/critical-section.h"

static void register_listener(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_event_t *listener
){
    uel_llist_t *listeners = &relay->signal_vector[signal];
    uel_llist_node_t *node = uel_syspools_acquire_llist_node(relay->pools);
    node->value = (void *)listener;

    UEL_CRITICAL_ENTER;
    uel_llist_push_head(listeners, node);
    UEL_CRITICAL_EXIT;
}

void uel_signal_relay_init(
    uel_signal_relay_t *relay,
    uel_syspools_t *pools,
    uel_sysqueues_t *queues,
    uel_llist_t *buffer,
    uintptr_t width
){
    relay->pools = pools;
    relay->queues = queues;
    relay->signal_vector = buffer;
    relay->width = width;

    for (uintptr_t i = 0; i < width; i++) {
        uel_llist_init(&relay->signal_vector[i]);
    }
}
uel_signal_listener_t uel_signal_listen(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_closure_t *closure
){
    uel_event_t *listener = uel_syspools_acquire_event(relay->pools);
    uel_event_config_signal_listener(listener, closure, true);
    register_listener(signal, relay, listener);
    return &listener->detail.listener;
}
uel_signal_listener_t uel_signal_listen_once(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_closure_t *closure
){
    uel_event_t *listener = uel_syspools_acquire_event(relay->pools);
    uel_event_config_signal_listener(listener, closure, false);
    register_listener(signal, relay, listener);
    return &listener->detail.listener;
}

void uel_signal_unlisten(uel_signal_listener_t listener){
    listener->unlistened = true;
}

void uel_signal_emit(uel_signal_t signal, uel_signal_relay_t *relay, void *params){
    uel_llist_t *listeners = &relay->signal_vector[signal];
    bool has_listeners = false;
    UEL_CRITICAL_ENTER;
    has_listeners = listeners->count > 0;
    UEL_CRITICAL_EXIT;
    if (has_listeners) {
        uel_event_t *event = uel_syspools_acquire_event(relay->pools);
        uel_event_config_signal(event, signal, listeners, params);
        uel_sysqueues_enqueue_event(relay->queues, event);
    }
}

uel_signal_listener_t uel_signal_resolve_promise(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_promise_t *promise
) {
    uel_closure_t resolver = uel_promise_resolver(promise);
    return uel_signal_listen_once(signal, relay, &resolver);
}


uel_signal_listener_t uel_signal_reject_promise(
    uel_signal_t signal,
    uel_signal_relay_t *relay,
    uel_promise_t *promise
) {
    uel_closure_t rejecter = uel_promise_rejecter(promise);
    return uel_signal_listen_once(signal, relay, &rejecter);
}
