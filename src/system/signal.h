#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdlib.h>
#include "event-loop.h"
#include "../utils/linked-list.h"
#include "../utils/object-pool.h"
#include "../utils/closure.h"

typedef uintptr_t signal_t;

typedef struct signal_listener signal_listener_t;
struct signal_listener{
    llist_t *source;
    llist_node_t *node;
};

typedef struct signal_relay signal_relay_t;
struct signal_relay{
    llist_t *signal_vector;
    evloop_t *event_loop;
    objpool_t *llist_node_pool;
    objpool_t *event_pool;
    uintptr_t width;
};

void signal_relay_init(
    signal_relay_t *relay,
    evloop_t *event_loop,
    objpool_t *llist_node_pool,
    objpool_t *event_pool,
    llist_t *buffer,
    uintptr_t width
);

signal_listener_t signal_listen(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
);

signal_listener_t signal_listen_once(
    signal_t signal,
    signal_relay_t *relay,
    closure_t *closure
);

void signal_unlisten(signal_listener_t listener, signal_relay_t *relay);

void signal_emit(signal_t signal, signal_relay_t *relay, void *params);

#endif /* end of include guard: SIGNAL_H */
