#include "linked-list.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "uevloop/utils/linked-list.h"
#include "../uelt.h"

static char *should_init_llist(){
    uel_llist_t list;
    uel_llist_init(&list);

    uelt_assert_pointer_null("llist.head", list.head);
    uelt_assert_pointer_null("llist.tail", list.tail);
    uelt_assert_int_zero("llist.count", list.count);

    return NULL;
}

static char *should_push_elements(){
    uel_llist_t list;
    uel_llist_init(&list);

    uel_llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };
    uel_llist_push_head(&list, &nodes[0]);
    uelt_assert_ints_equal("llist.count with one element", 1, list.count);

    uel_llist_push_head(&list, &nodes[1]);
    uelt_assert_ints_equal("llist.count with two elements", 2, list.count);

    uel_llist_push_tail(&list, &nodes[2]);
    uelt_assert_ints_equal("llist.count with three elements", 3, list.count);

    uel_llist_node_t *node = list.tail;
    uelt_assert_pointers_equal("llist.tail", &nodes[2], node);
    uelt_assert_ints_equal("llist.tail->value", nodes[2].value, node->value);

    node = node->next;
    uelt_assert_pointers_equal("llist.tail->next", &nodes[0], node);
    uelt_assert_ints_equal("llist.tail->next->value", nodes[0].value, node->value);

    node = node->next;
    uelt_assert_pointers_equal("llist.tail->next->next", &nodes[1], node);
    uelt_assert_pointers_equal("llist.head", node, list.head);
    uelt_assert_ints_equal("llist.tail->next->next->value", nodes[1].value, node->value);

    return NULL;
}

static char *should_pop_elements(){
    uel_llist_t list;
    uel_llist_init(&list);

    uel_llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };
    uel_llist_push_head(&list, &nodes[0]);
    uel_llist_push_head(&list, &nodes[1]);
    uel_llist_push_tail(&list, &nodes[2]);

    uel_llist_node_t *node;
    node = uel_llist_pop_head(&list);
    uelt_assert_pointers_equal("first popped element", &nodes[1], node);
    uelt_assert_ints_equal("llist.count after first element popped", 2, list.count);

    node = uel_llist_pop_tail(&list);
    uelt_assert_pointers_equal("second popped element", &nodes[2], node);
    uelt_assert_ints_equal("llist.count after second element popped", 1, list.count);

    node = uel_llist_pop_head(&list);
    uelt_assert_pointers_equal("third popped element", &nodes[0], node);
    uelt_assert_ints_equal("llist.count after third element popped", 0, list.count);

    return NULL;
}

static char *should_peek_elements(){
    uel_llist_t list;
    uel_llist_init(&list);

    uel_llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };
    uel_llist_push_head(&list, &nodes[0]);
    uel_llist_push_head(&list, &nodes[1]);
    uel_llist_push_tail(&list, &nodes[2]);

    uel_llist_node_t *head = uel_llist_peek_head(&list);
    uelt_assert_ints_equal("llist.count", 3, list.count);
    uelt_assert_pointers_equal("uel_llist_peek_head()", &nodes[1], head);
    uelt_assert_pointers_equal("llist.head", head, list.head);

    uel_llist_node_t *tail = uel_llist_peek_tail(&list);
    uelt_assert_ints_equal("llist.count", 3, list.count);
    uelt_assert_pointers_equal("uel_llist_peek_tail()", &nodes[2], tail);
    uelt_assert_pointers_equal("llist.tail", tail, list.tail);

    return NULL;
}

static void *less_than(uel_closure_t *closure){
    uintptr_t threshold = (uintptr_t)closure->context;
    uel_llist_node_t *node = (uel_llist_node_t *)closure->params;
    uintptr_t value = (uintptr_t)node->value;
    return (void*)(uintptr_t)(value < threshold);
}
static char *should_remove_elements_until_condition(){
    uel_llist_t list;
    uel_llist_init(&list);

    uel_llist_node_t nodes[7] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
        { (void *)4, NULL },
        { (void *)5, NULL },
        { (void *)6, NULL },
        { (void *)7, NULL }
    };

    for(uintptr_t i = 0; i < 7; i++) uel_llist_push_head(&list, &nodes[i]);

    uel_closure_t less_than_five = uel_closure_create(&less_than, (void *)5, NULL);
    uel_llist_t removed = uel_llist_remove_while(&list, &less_than_five);

    uelt_assert_ints_equal("list.count", 3, list.count);
    uelt_assert_pointers_equal("list.head", &nodes[6], list.head);
    uelt_assert_pointers_equal("list.tail", &nodes[4], list.tail);

    uelt_assert_pointers_equal("removed.head", &nodes[3], removed.head);
    uelt_assert_pointers_equal("removed.tail", &nodes[0], removed.tail);
    uelt_assert_ints_equal("removed.count", 4, removed.count);

    return NULL;
}


static void *between_ranges(uel_closure_t *closure){
    uel_llist_node_t **nodes = (uel_llist_node_t **)closure->params;
    bool fits = false;

    if(nodes[1] == NULL){
        fits = true;
    }else if(nodes[0] != NULL){
        uintptr_t value1 = (uintptr_t)nodes[0]->value;
        uintptr_t value2 = (uintptr_t)nodes[1]->value;
        fits = value1 < value2;
    }
    return (void *)(uintptr_t)fits;

}
static char *should_insert_element_when_condition(){
    uel_llist_t list;
    uel_llist_init(&list);

    uel_llist_node_t nodes[6] = {
        { (void *)1, NULL },
        { (void *)1, NULL },
        { (void *)1, NULL },
        { (void *)3, NULL },
        { (void *)3, NULL },
        { (void *)3, NULL }
    };
    for(uintptr_t i = 0; i < 6; i++) uel_llist_push_head(&list, &nodes[i]);

    uel_llist_node_t node = { (void *)2, NULL };
    uel_closure_t is_between_ranges = uel_closure_create(&between_ranges, NULL, NULL);
    uel_llist_insert_at(&list, &node, &is_between_ranges);

    uelt_assert_ints_equal("list.count", 7, list.count);
    uelt_assert_pointers_equal("list.head", &nodes[5], list.head);
    uelt_assert_pointers_equal("list.tail", &nodes[0], list.tail);
    uelt_assert_pointers_equal("nodes[2].next", &node, nodes[2].next);
    uelt_assert_pointers_equal("node.next", &nodes[3], node.next);


    return NULL;
}

static bool contains(uel_llist_t *list, uel_llist_node_t *node){
    uel_llist_node_t *current = list->tail;
    while(current != NULL){
        if (current == node) return true;
        current = current->next;
    }
    return false;
}
static char *should_remove_elements(){
    uel_llist_t list;
    uel_llist_init(&list);

    uel_llist_node_t node1 = { (void *)1, NULL };
    uel_llist_node_t node2 = { (void *)2, NULL };
    uel_llist_node_t node3 = { (void *)2, NULL };
    uel_llist_push_head(&list, &node1);
    uel_llist_push_head(&list, &node2);
    uel_llist_push_head(&list, &node3);

    uelt_assert_ints_equal("list.count", 3, list.count);

    uel_llist_remove(&list, &node2);
    uelt_assert_ints_equal("list.count after first removal", 2, list.count);
    uelt_assert_not("list must not contain node2", contains(&list, &node2));

    uel_llist_remove(&list, &node3);
    uelt_assert_ints_equal("list.count after second removal", 1, list.count);
    uelt_assert_not("list must not contain node3", contains(&list, &node3));

    uel_llist_remove(&list, &node1);
    uelt_assert_int_zero("list.count after third removal", list.count);
    uelt_assert_not("list must not contain node1", contains(&list, &node1));

    return NULL;
}

char *uel_llist_run_tests(){
    uelt_run_test("should correctly initialise a linked list", should_init_llist);
    uelt_run_test("should push elements to a linked list", should_push_elements);
    uelt_run_test("should pop elements from a linked list", should_pop_elements);
    uelt_run_test("should peek elements from a linked list", should_peek_elements);
    uelt_run_test(
        "should remove elements into a new list until condition is met",
        should_remove_elements_until_condition
    );
    uelt_run_test(
        "should insert an element into a list at the position some condition is met",
        should_insert_element_when_condition
    );
    uelt_run_test(
        "should remove arbitraty elements",
        should_remove_elements
    );

    return NULL;
}
