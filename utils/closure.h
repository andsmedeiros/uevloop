/* 
 * File:   closure.h
 * Author: kazeshi
 *
 * Created on January 3, 2020, 10:54 AM
 */

#ifndef CLOSURE_H
#define	CLOSURE_H

typedef struct closure closure_t;
struct closure{
    void (* function)(closure_t *);
    void * context;
    void (* destructor)(closure_t *);
    void * params;
    void * value;
};

closure_t closure_create(
    void (* function)(closure_t *), 
    void *context,
    void (* destructor)(closure_t *)
);
void *closure_invoke(closure_t *closure, void *params);
void closure_destroy(closure_t *closure);
void closure_return(closure_t *closure, void *value);

#endif	/* CLOSURE_H */

