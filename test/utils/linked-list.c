#include "linked-list.h"

#include <stdlib.h>

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
    llist_init(&llist);

    llist_node_t nodes[3] = {
        { (void *)1, NULL },
        { (void *)2, NULL },
        { (void *)3, NULL },
    };

    return NULL;
}

char *llist_run_tests(){

    mu_run_test("should correctly initialise a linked list", should_init_llist);
    mu_run_test("should push elements to a linked list", should_push_elements);

    return NULL;
}
