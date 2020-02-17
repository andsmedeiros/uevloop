#include "signal.h"

/// \cond
#include <stdbool.h>
#include <stdlib.h>
/// \endcond

#include "event.h"
#include "../config.h"
#include "../portability/critical-section.h"

static signal_listener_t register_event(
    signal_t signal,
    signal_relay_t *relay,
    event_t *event
){
    signal_listener_t listener;
    llist_t *listeners = &relay->signal_vector[signal];
    listener.source = listeners;

    llist_node_t *node = syspools_acquire_llist_node(relay->pools);
    node->value = (void *)event;
    listener.node = node;

    UEVLOOP_CRITICAL_ENTER;
    llist_push_head(listeners, node);
    UEVLOOP_CRITICAL_EXIT;

    return listener;
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
    event_t *event = syspools_acquire_event(relay->pools);
    event_config_signal(event, true, closure);
    return register_event(signal, relay, event);
}

signal_listener_t signal_listen_once(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
){
    event_t *event = syspools_acquire_event(relay->pools);
    event_config_signal(event, false, closure);
    return register_event(signal, relay, event);
}

void signal_unlisten(signal_listener_t listener, signal_relay_t *relay){
    bool removed;
    UEVLOOP_CRITICAL_ENTER;
    removed = llist_remove(listener.source, listener.node);
    UEVLOOP_CRITICAL_EXIT;

    if (removed) {
        event_t *event = (event_t *)listener.node->value;
        event_destroy(event);
        syspools_release_event(relay->pools, event);
        syspools_release_llist_node(relay->pools, listener.node);
    }
}

#include <stdio.h>

void signal_emit(signal_t signal, signal_relay_t *relay, void *params){
    closure_t closures[SIGNAL_MAX_LISTENERS];
    llist_node_t *nodes_for_removal[SIGNAL_MAX_LISTENERS];
    llist_t *listeners = &relay->signal_vector[signal];
    unsigned int i = 0, j =  0;

    UEVLOOP_CRITICAL_ENTER;
    for(llist_node_t *current = listeners->tail;
        current != NULL && i < SIGNAL_MAX_LISTENERS;
        current = current->next, i++
    ){
        event_t *event = (event_t *)current->value;
        if(!event->repeating){
            llist_remove(listeners, current);
            nodes_for_removal[j++] = current;
        }
        closures[i] = event->closure;
    }
    UEVLOOP_CRITICAL_EXIT;

    for(unsigned int closure_count = i, i = 0; i < closure_count; i++){
        event_t *event = syspools_acquire_event(relay->pools);
        closure_t *closure = &closures[i];
        closure->params = params;
        event_config_signal(event, false, closure);
        sysqueues_enqueue_event(relay->queues, event);
    }
    for(unsigned int node_count = j, j = 0; j < node_count; j++){
        event_t *event = nodes_for_removal[j]->value;
        event_destroy(event);
        syspools_release_event(relay->pools, event);
        syspools_release_llist_node(relay->pools, nodes_for_removal[j]);
    }
}
