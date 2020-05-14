#include "uevloop/utils/iterator.h"

typedef void * (* uel_iterator_next_t)(uel_iterator_t *, void *);

bool uel_iterator_foreach(uel_iterator_t *iterator, uel_closure_t *closure){
    void *last = NULL;
    while(1){
        last = iterator->next(iterator, last);
        if(last == NULL){
            return true;
        }else if(!uel_closure_invoke(closure, last)){
            return false;
        }
    }
}

size_t uel_iterator_map(uel_iterator_t *iterator, uel_closure_t *closure, void **destination, size_t limit){
    void *last = NULL;
    for(size_t i = 0; i < limit; i++){
        last = iterator->next(iterator, last);
        if(last == NULL){
            return i;
        }else{
            destination[i] = uel_closure_invoke(closure, last);
        }
    }
    return limit;
}

void *uel_iterator_find(uel_iterator_t *iterator, uel_closure_t *closure){
    void *last = NULL;
    while(1){
        last = iterator->next(iterator, last);
        if(last == NULL){
            return NULL;
        }else if(uel_closure_invoke(closure, last)){
            return last;
        }
    }
}

size_t uel_iterator_count(uel_iterator_t *iterator, uel_closure_t *closure){
    void *last = NULL;
    size_t count = 0;
    while (1) {
        last = iterator->next(iterator, last);
        if (last == NULL) {
            return count;
        }else if(uel_closure_invoke(closure, last)){
            count++;
        }
    }
}

bool uel_iterator_all(uel_iterator_t *iterator, uel_closure_t *closure){
    void *last = NULL;
    while(1){
        last = iterator->next(iterator, last);
        if(last == NULL){
            return true;
        }else if(!uel_closure_invoke(closure, last)){
            return false;
        }
    }
}

bool uel_iterator_none(uel_iterator_t *iterator, uel_closure_t *closure){
    return !uel_iterator_any(iterator, closure);
}

bool uel_iterator_any(uel_iterator_t *iterator, uel_closure_t *closure){
    void *last = NULL;
    while(1){
        last = iterator->next(iterator, last);
        if(last == NULL){
            return false;
        }else if(uel_closure_invoke(closure, last)){
            return true;
        }
    }
}

void *uel_iterator_array_next(uel_iterator_array_t *iterator, void *last){
    if(last == NULL){
        return iterator->base.collection;
    }else{
        void *boundary =
            (char *)iterator->base.collection +
            (iterator->item_count - 1) * iterator->item_size;
        if(last == boundary){
            return NULL;
        }
        return (char *)last + iterator->item_size;
    }
}

uel_iterator_array_t uel_iterator_array_create(void *collection, size_t count, size_t size){
    uel_iterator_array_t iterator = {
        { (uel_iterator_next_t)&uel_iterator_array_next, collection },
        count,
        size
    };
    return iterator;
}

void *uel_iterator_llist_next(uel_iterator_t *iterator, void *last){
    if(last == NULL){
        uel_llist_t *list = (uel_llist_t *)iterator->collection;
        return (void *)list->tail;
    }else{
        uel_llist_node_t *node = (uel_llist_node_t *)last;
        return (void *)node->next;
    }
}

uel_iterator_llist_t uel_iterator_llist_create(uel_llist_t *list){
    uel_iterator_llist_t iterator = {uel_iterator_llist_next, (void *)list};
    return iterator;
}
