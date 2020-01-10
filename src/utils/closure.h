#ifndef CLOSURE_H
#define	CLOSURE_H

typedef struct closure closure_t;
struct closure{
    void * (* function)(closure_t *);
    void *context;
    void (* destructor)(closure_t *);
    void *params;
    void *value;
};

closure_t closure_create(
    void * (* function)(closure_t *),
    void *context,
    void (* destructor)(closure_t *)
);
void *closure_invoke(closure_t *closure, void *params);
void closure_destroy(closure_t *closure);

#endif	/* CLOSURE_H */
