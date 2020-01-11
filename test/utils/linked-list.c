#include "linked-list.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils/linked-list.h"
#include "../minunit.h"

static char *should_init_llist(){
    llist_t list;
    llist_init(&list);

    mu_assert_pointer_null("llist.head", list.head);
    mu_assert_pointer_null("llist.tail", list.tail);
    mu_assert_int_zero("llist.count", list.count);

    return NULL;
}

static char *should_push_elements(){
    llist_t list;
    llist_init(&list);

    llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };
    llist_push_head(&list, &nodes[0]);
    mu_assert_ints_equal("llist.count with one element", 1, list.count);

    llist_push_head(&list, &nodes[1]);
    mu_assert_ints_equal("llist.count with two elements", 2, list.count);

    llist_push_tail(&list, &nodes[2]);
    mu_assert_ints_equal("llist.count with three elements", 3, list.count);

    llist_node_t *node = list.tail;
    mu_assert_pointers_equal("llist.tail", &nodes[2], node);
    mu_assert_ints_equal("llist.tail->value", nodes[2].value, node->value);

    node = node->next;
    mu_assert_pointers_equal("llist.tail->next", &nodes[0], node);
    mu_assert_ints_equal("llist.tail->next->value", nodes[0].value, node->value);

    node = node->next;
    mu_assert_pointers_equal("llist.tail->next->next", &nodes[1], node);
    mu_assert_pointers_equal("llist.head", node, list.head);
    mu_assert_ints_equal("llist.tail->next->next->value", nodes[1].value, node->value);

    return NULL;
}

static char *should_pop_elements(){
    llist_t list;
    llist_init(&list);

    llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };
    llist_push_head(&list, &nodes[0]);
    llist_push_head(&list, &nodes[1]);
    llist_push_tail(&list, &nodes[2]);

    llist_node_t *node;
    node = llist_pop_head(&list);
    mu_assert_pointers_equal("first popped element", &nodes[1], node);
    mu_assert_ints_equal("llist.count after first element popped", 2, list.count);

    node = llist_pop_tail(&list);
    mu_assert_pointers_equal("second popped element", &nodes[2], node);
    mu_assert_ints_equal("llist.count after second element popped", 1, list.count);

    node = llist_pop_head(&list);
    mu_assert_pointers_equal("third popped element", &nodes[0], node);
    mu_assert_ints_equal("llist.count after third element popped", 0, list.count);

    return NULL;
}

static char *should_peek_elements(){
    llist_t list;
    llist_init(&list);

    llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };
    llist_push_head(&list, &nodes[0]);
    llist_push_head(&list, &nodes[1]);
    llist_push_tail(&list, &nodes[2]);

    llist_node_t *head = llist_peek_head(&list);
    mu_assert_ints_equal("llist.count", 3, list.count);
    mu_assert_pointers_equal("llist_peek_head()", &nodes[1], head);
    mu_assert_pointers_equal("llist.head", head, list.head);

    llist_node_t *tail = llist_peek_tail(&list);
    mu_assert_ints_equal("llist.count", 3, list.count);
    mu_assert_pointers_equal("llist_peek_tail()", &nodes[2], tail);
    mu_assert_pointers_equal("llist.tail", tail, list.tail);

    return NULL;
}

static void *less_than(closure_t *closure){
    uintptr_t threshold = (uintptr_t)closure->context;
    llist_node_t *node = (llist_node_t *)closure->params;
    uintptr_t value = (uintptr_t)node->value;
    return (void*)(uintptr_t)(value < threshold);
}
static char *should_remove_elements_until_condition(){
    llist_t list;
    llist_init(&list);

    llist_node_t nodes[7] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
        { (void *)4, NULL },
        { (void *)5, NULL },
        { (void *)6, NULL },
        { (void *)7, NULL }
    };

    for(uintptr_t i = 0; i < 7; i++) llist_push_head(&list, &nodes[i]);

    closure_t less_than_five = closure_create(&less_than, (void *)5, NULL);
    llist_t removed = llist_remove_until(&list, &less_than_five);

    mu_assert_ints_equal("list.count", 3, list.count);
    mu_assert_pointers_equal("list.head", &nodes[6], list.head);
    mu_assert_pointers_equal("list.tail", &nodes[4], list.tail);

    mu_assert_pointers_equal("removed.head", &nodes[3], removed.head);
    mu_assert_pointers_equal("removed.tail", &nodes[0], removed.tail);
    mu_assert_ints_equal("removed.count", 4, removed.count);

    return NULL;
}


static void *between_ranges(closure_t *closure){
    llist_node_t **nodes = (llist_node_t **)closure->params;
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
    llist_t list;
    llist_init(&list);

    llist_node_t nodes[6] = {
        { (void *)1, NULL },
        { (void *)1, NULL },
        { (void *)1, NULL },
        { (void *)3, NULL },
        { (void *)3, NULL },
        { (void *)3, NULL }
    };
    for(uintptr_t i = 0; i < 6; i++) llist_push_head(&list, &nodes[i]);

    llist_node_t node = { (void *)2, NULL };
    closure_t is_between_ranges = closure_create(&between_ranges, NULL, NULL);
    llist_insert_at(&list, &node, &is_between_ranges);

    mu_assert_ints_equal("list.count", 7, list.count);
    mu_assert_pointers_equal("list.head", &nodes[5], list.head);
    mu_assert_pointers_equal("list.tail", &nodes[0], list.tail);
    mu_assert_pointers_equal("nodes[2].next", &node, nodes[2].next);
    mu_assert_pointers_equal("node.next", &nodes[3], node.next);


    return NULL;
}

static bool contains(llist_t *list, llist_node_t *node){
    llist_node_t *current = list->tail;
    while(current != NULL){
        if (current == node) return true;
        current = current->next;
    }
    return false;
}
static char *should_remove_elements(){
    llist_t list;
    llist_init(&list);

    llist_node_t node1 = { (void *)1, NULL };
    llist_node_t node2 = { (void *)2, NULL };
    llist_node_t node3 = { (void *)2, NULL };
    llist_push_head(&list, &node1);
    llist_push_head(&list, &node2);
    llist_push_head(&list, &node3);

    mu_assert_ints_equal("list.count", 3, list.count);

    llist_remove(&list, &node2);
    mu_assert_ints_equal("list.count after first removal", 2, list.count);
    mu_assert_not("list must not contain node2", contains(&list, &node2));

    llist_remove(&list, &node3);
    mu_assert_ints_equal("list.count after second removal", 1, list.count);
    mu_assert_not("list must not contain node3", contains(&list, &node3));

    llist_remove(&list, &node1);
    mu_assert_int_zero("list.count after third removal", list.count);
    mu_assert_not("list must not contain node1", contains(&list, &node1));

    return NULL;
}

char *llist_run_tests(){
    mu_run_test("should correctly initialise a linked list", should_init_llist);
    mu_run_test("should push elements to a linked list", should_push_elements);
    mu_run_test("should pop elements from a linked list", should_pop_elements);
    mu_run_test("should peek elements from a linked list", should_peek_elements);
    mu_run_test(
        "should remove elements into a new list until condition is met",
        should_remove_elements_until_condition
    );
    mu_run_test(
        "should insert an element into a list at the position some condition is met",
        should_insert_element_when_condition
    );
    mu_run_test(
        "should remove arbitraty elements",
        should_remove_elements
    );

    return NULL;
}
