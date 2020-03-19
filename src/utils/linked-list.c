#include "linked-list.h"

/// \cond
#include <stdbool.h>
#include <stdlib.h>
/// \endcond

void uel_llist_init(uel_llist_t *list){
    list->head = list->tail = NULL;
    list->count = 0;
}

void uel_llist_push_head(uel_llist_t *list, uel_llist_node_t *node){
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

void uel_llist_push_tail(uel_llist_t *list, uel_llist_node_t *node){
    node->next = list->tail;
    if(list->head == NULL){
        list->head = node;
    }
    list->tail = node;
    list->count++;
}

uel_llist_node_t *uel_llist_pop_head(uel_llist_t *list){
    if(list->head == NULL) return NULL;

    uel_llist_node_t *current = list->tail, *head = list->head;
    while(current->next != head && current->next != NULL){
        current = current->next;
    }
    current->next = NULL;
    list->head = current;
    list->count--;
    return head;
}

uel_llist_node_t *uel_llist_pop_tail(uel_llist_t *list){
    if(list->tail == NULL) return NULL;

    uel_llist_node_t *tail = list->tail;
    list->tail = list->tail->next;
    list->count--;
    return tail;
}

uel_llist_node_t *uel_llist_peek_head(uel_llist_t *list){
    return list->head;
}
uel_llist_node_t *uel_llist_peek_tail(uel_llist_t *list){
    return list->tail;
}

bool uel_llist_remove(uel_llist_t *list, uel_llist_node_t *node){
    if(node == list->tail){
        list->tail = node->next;
        list->count--;
        return true;
    }

    uel_llist_node_t *current = list->tail;
    while(current != NULL){
        if(current->next == node){
            current->next = node->next;
            list->count--;
            return true;
        }
        current = current->next;
    }

    return false;
}

uel_llist_t uel_llist_remove_while(uel_llist_t *list, uel_closure_t *should_remove){
    uel_llist_t removed;
    uel_llist_init(&removed);

    uel_llist_node_t *current;
    while((current = uel_llist_peek_tail(list)) != NULL){
        bool fit_for_removal = (bool)uel_closure_invoke(should_remove, current);
        if(fit_for_removal){
            current = uel_llist_pop_tail(list);
            uel_llist_push_head(&removed, current);
        }else break;
    }
    return removed;
}

void uel_llist_insert_at(uel_llist_t *list, uel_llist_node_t *node, uel_closure_t *should_insert){
    uel_llist_node_t *current = uel_llist_peek_tail(list);

    if(current == NULL){
        uel_llist_push_tail(list, node);
    }else{
        uel_llist_node_t *nodes[2] = { NULL, current };
        bool fit_for_insertion = (bool)uel_closure_invoke(should_insert, (void *)&nodes);

        if(fit_for_insertion){
            uel_llist_push_tail(list, node);
        }else{
            while(current != NULL && !fit_for_insertion){
                nodes[0] = current;
                nodes[1] = current->next;
                fit_for_insertion = (bool)uel_closure_invoke(should_insert, (void *)&nodes);
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

}
