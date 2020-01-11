#include "signal.h"

#include <stdbool.h>
#include "event.h"

static signal_listener_t register_event(
    signal_t signal,
    signal_relay_t *relay,
    event_t *event
){
    signal_listener_t listener;
    llist_t *listeners = &relay->signal_vector[signal];
    listener.source = listeners;

    llist_node_t *node = (llist_node_t *)objpool_acquire(relay->llist_node_pool);
    node->value = (void *)event;
    listener.node = node;

    llist_push_head(listeners, node);

    return listener;
}

void signal_relay_init(
    signal_relay_t *relay,
    evloop_t *event_loop,
    objpool_t *llist_node_pool,
    objpool_t *event_pool,
    llist_t *buffer,
    uintptr_t width
){
    relay->event_loop = event_loop;
    relay->llist_node_pool = llist_node_pool;
    relay->event_pool = event_pool;
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
    event_t *event = (event_t *)objpool_acquire(relay->event_pool);
    event_config_signal(event, true, closure);
    return register_event(signal, relay, event);
}

signal_listener_t signal_listen_once(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
){
    event_t *event = (event_t *)objpool_acquire(relay->event_pool);
    event_config_signal(event, false, closure);
    return register_event(signal, relay, event);
}

void signal_unlisten(signal_listener_t listener, signal_relay_t *relay){
    llist_remove(listener.source, listener.node);
    event_t *event = (event_t *)listener.node->value;
    event_destroy(event);
    objpool_release(relay->event_pool, (void *)event);
    objpool_release(relay->llist_node_pool, (void *)listener.node);
}

void signal_emit(signal_t signal, signal_relay_t *relay, void *params){
    llist_t *listeners = &relay->signal_vector[signal];
    llist_node_t *current = listeners->tail;
    while(current != NULL){
        event_t *event = (event_t *)current->value;
        event->closure.params = params;
        evloop_enqueue_event(relay->event_loop, event);
        if(!event->repeating){
            signal_listener_t listener = { listeners, current };
            signal_unlisten(listener, relay);
        }
        current = current->next;
    }
}
