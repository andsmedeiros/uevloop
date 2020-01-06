#include "linked-list.h"
#include <stdbool.h>

void llist_init(llist_t *list){
    list->head = list->tail = NULL;
    list->count = 0;
}

void llist_push_head(llist_t *list, llist_node_t *node){
    node->next = NULL;
    if(list->head != NULL){
        list->head->next = node;
    }
    if(list->tail == NULL){
       list->tail = node;
    }
    list->head = node;
    list->count++;
}

void llist_push_tail(llist_t *list, llist_node_t *node){
    node->next = list->tail;
    if(list->head == NULL){
        list->head = node;
    }
    list->tail = node;
    list->count++;
}

llist_node_t *llist_pop_head(llist_t *list){
    if(list->head == NULL) return NULL;

    llist_node_t *current = list->tail, *head = list->head;
    while(current->next != head && current->next != NULL){
        current = current->next;
    }
    current->next = NULL;
    list->head = current;
    list->count--;
    return head;
}

llist_node_t *llist_pop_tail(llist_t *list){
    if(list->tail == NULL) return NULL;

    llist_node_t *tail = list->tail;
    list->tail = list->tail->next;
    list->count--;
    return tail;
}

llist_node_t *llist_peek_head(llist_t *list){
    return list->head;
}
llist_node_t *llist_peek_tail(llist_t *list){
    return list->tail;
}

llist_t llist_remove_until(llist_t *list, closure_t *should_remove){
    llist_t removed;
    llist_init(&removed);

    llist_node_t *current;
    while((current = llist_peek_tail(list)) != NULL){
        bool fit_for_removal = (bool)closure_invoke(should_remove, current);
        if(fit_for_removal){
            current = llist_pop_tail(list);
            llist_push_head(&removed, current);
        }else break;
    }
    return removed;
}

void llist_insert_at(llist_t *list, llist_node_t *node, closure_t *should_insert){
    llist_node_t *current = llist_peek_tail(list);

    if(current == NULL){
        llist_push_tail(list, node);
    }else{
        while(current != NULL){
            llist_node_t *nodes[2] = { current, current->next };
            bool fit_for_insertion = (bool)closure_invoke(should_insert, (void *)&nodes);
            if(fit_for_insertion){
                node->next = current->next;
                current->next = node;
                list->count++;
                return;
            }
            current = current->next;
        }
    }

}
