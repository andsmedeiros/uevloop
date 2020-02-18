#include "signal.h"

/// \cond
#include <stdbool.h>
#include <stdlib.h>
/// \endcond

#include "../config.h"
#include "../portability/critical-section.h"

static void register_listener(
    signal_t signal,
    signal_relay_t *relay,
    event_t *listener
){
    llist_t *listeners = &relay->signal_vector[signal];
    llist_node_t *node = syspools_acquire_llist_node(relay->pools);
    node->value = (void *)listener;

    UEVLOOP_CRITICAL_ENTER;
    llist_push_head(listeners, node);
    UEVLOOP_CRITICAL_EXIT;
}

void signal_relay_init(
    signal_relay_t *relay,
    syspools_t *pools,
    sysqueues_t *queues,
    llist_t *buffer,
    uintptr_t width
){
    relay->pools = pools;
    relay->queues = queues;
    relay->signal_vector = buffer;
    relay->width = width;

    for (uintptr_t i = 0; i < width; i++) {
        llist_init(&relay->signal_vector[i]);
    }
}

signal_listener_t signal_listen(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
){
    event_t *listener = syspools_acquire_event(relay->pools);
    event_config_signal_listener(listener, closure, true);
    register_listener(signal, relay, listener);
    return &listener->detail.listener;
}

signal_listener_t signal_listen_once(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
){
    event_t *listener = syspools_acquire_event(relay->pools);
    event_config_signal_listener(listener, closure, false);
    register_listener(signal, relay, listener);
    return &listener->detail.listener;
}

void signal_unlisten(signal_listener_t listener){
    listener->unlistened = true;
}

void signal_emit(signal_t signal, signal_relay_t *relay, void *params){
    llist_t *listeners = &relay->signal_vector[signal];
    bool has_listeners = false;
    UEVLOOP_CRITICAL_ENTER;
    has_listeners = listeners->count > 0;
    UEVLOOP_CRITICAL_EXIT;
    if (has_listeners) {
        event_t *event = syspools_acquire_event(relay->pools);
        event_config_signal(event, signal, listeners, params);
        sysqueues_enqueue_event(relay->queues, event);
    }
}
