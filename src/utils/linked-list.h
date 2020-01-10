#ifndef LINKED_LIST_H
#define	LINKED_LIST_H

#include <stdlib.h>
#include "closure.h"

typedef struct llist_node llist_node_t;
struct llist_node{
    void *value;
    llist_node_t *next;
};

typedef struct llist llist_t;
struct llist{
    llist_node_t *head;
    llist_node_t *tail;
    size_t count;
};

void llist_init(llist_t *list);
void llist_push_head(llist_t *list, llist_node_t *node);
void llist_push_tail(llist_t *list, llist_node_t *node);
llist_node_t *llist_pop_head(llist_t *list);
llist_node_t *llist_pop_tail(llist_t *list);
llist_node_t *llist_peek_head(llist_t *list);
llist_node_t *llist_peek_tail(llist_t *list);

void llist_remove(llist_t *list, llist_node_t *node);

llist_t llist_remove_until(llist_t *list, closure_t *should_remove);
void llist_insert_at(llist_t *list, llist_node_t *node, closure_t *should_insert);

#endif	/* LINKED_LIST_H */
