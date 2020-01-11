# µEvLoop

A fast and lightweight event loop aimed at embedded platforms in C99.

## About

µEvLoop is a microframework build around a lightweight event loop. It provides the programmer the building blocks to put together async, interrupt-based systems.

µEvLoop is loosely inspired on the Javascript event loop and aims to provide a similar programming model. Many similar concepts, such as events and closures are included.

Because it aims at embedded or platforms with very few resources, all memory must be statically allocated by the user. The library per se does **NOT** allocate any memory on its own.

### *DISCLAIMER*

µEvLoop is in its early days and the API may change at any moment for now. Although it's well tested, use it with caution. Anyway, feedback is most welcome.

## Core data structures

### Closures

A closure is an object that binds a function to some context. It call be called with appropriate parameters then, just like a regular function, when both context and parameters will be available. With closures, some very powerful programming patterns, as functional composition, becomes way easier to implement.

As closures are somewhat light, it is often useful to pass it around by value.

#### Basic closure usage
```C
  #include <stdlib.h>
  #include <stdint.h>
  #include "utils/closure.h"

  static void *add(closure_t *closure){
    uintptr_t value1 = (uintptr_t)closure->context;
    uintptr_t value2 = (uintptr_t)closure->params;

    return (void *)(value1 + value2);
  }

  // ...

  // Binds the function `add` to the context (5)
  closure_t add_five = closure_create(&add, (void *)5, NULL);

  // Invokes the closure with the parameters set to (2)
  uintptr_t result = (uintptr_t)closure_invoke(&add_five, (void *)2);
  // Result is 7

```

#### *A word on (void *)*

Closures take the context and params as a void pointer and return the same. This is meant to make possible to pass and return complex objects from them.

At many times, however, you may find the values passed/returned are small and simple (*i.e.*: smaller than a pointer). If so, it is absolutely valid to cast from/to a uintptr_t or other data type known to be at most the size of a pointer. The above example does that so we don't have to create unnecessary object pools or allocate dynamic memory.

### Object pools

The programming model implemented by µEvLoop requires a variable number of events enqueued. Because allocating **any** memory within the core code hurts the project guidelines and because it is *discouraged* from programmers to dynamically allocate memory, other dynamic object management solution is necessary.

Object pools are statically allocated arrays of objects whose addresses are stored in a queue. Whenever the programmer needs a dynamic object, instead of allocating memory it is possible to simply pop and object pointer from the pool and use it away.

#### Basic object pool usage

```c
#include <stdlib.h>
#include <stdint.h>
#include "utils/object-pool.h"

typedef struct obj obj_t;
struct obj {
  uint32_t num;
  char str[32];
  // Whatever
};

// ...

// The log2 of our pool size.
#define POOL_SIZE_LOG2N   (5)
DECLARE_OBJPOOL_BUFFERS(obj_t, POOL_SIZE_LOG2N, my_pool);

objpool_t my_pool;
objpool_init(&my_pool, POOL_SIZE_LOG2N, sizeof(obj_t), OBJPOOL_BUFFERS(my_pool));
// my_pool now is a pool with 32 (2**5) obj_t

// ...

// Whenever the programmer needs a fresh obj_t
obj_t *obj = (obj_t *)objpool_acquire(&my_pool);

// When it is no longer needed, return it to the pool
objpool_release(&my_pool, obj);

```

To understand why size is declared in log2, read along the next section.

### Circular queues

Circular queues are fast FIFO structures that rely on a pair of indices to maintain state. As the indices are moved forward on push/pop operations, the data itself is not moved at all.

The size of µEvLoop's circular queues are **required** to be powers of two, so it is possible to use fast modulo-2 arithmetics. As such, on queue creation, the size **must** be provided in its log2 form.

***FORGETTING TO SUPPLY THE QUEUE'S SIZE IN LOG2 FORM MAY CAUSE THE STATIC ALLOCATION OF GIANT MEMORY POOLS***

#### Basic circular queue usage

```c
#include <stdlib.h>
#include <stdint.h>
#include "utils/circular-queue.h"

#define BUFFER_SIZE_LOG2N   (5)
#define BUFFER_SIZE         (1<<BUFFER_SIZE_LOG2N)

// ...

cqueue_t queue;
void *buffer[BUFFER_SIZE];
// Created a queue with 32 (2**5) slots
cqueue_init(&queue, buffer, BUFFER_SIZE_LOG2N);

// Push items in the queue
cqueue_push(&queue, (void *)3);
cqueue_push(&queue, (void *)2);
cqueue_push(&queue, (void *)1);

// Pop items from the queue
uintptr_t value1 = (uintptr_t)cqueue_pop(&queue); // value1 is 3
uintptr_t value2 = (uintptr_t)cqueue_pop(&queue); // value2 is 2
uintptr_t value3 = (uintptr_t)cqueue_pop(&queue); // value3 is 1
```

Circular queues store only void pointers. As it is the case with closures, this make possible to store complex objects within the queue, but often typecasting to an smaller value type is more useful.

### Linked lists

µEvLoop ships a simple linked list implementation thar holds void pointers, as usual.

#### Basic linked list usage

```c
#include <stdlib.h>
#include <stdint.h>
#include "utils/linked-list.h"

// ...

llist_t list;
llist_init(&list);

llist_node_t nodes[2] = {
  {(void *)1, NULL},
  {(void *)2, NULL}
};

// Push items into the list
llist_push_head(&list, &nodes[0]);
llist_push_head(&list, &nodes[1]);

// List now is TAIL-> [1]-> [2]-> NULL. HEAD-> [2]
llist_node_t *node1 = (llist_node_t *)llist_pop_tail(&list);
llist_node_t *node2 = (llist_node_t *)llist_pop_tail(&list);

//node1 == nodes[0] and node2 == nodes[1]

```
## Core modules

### Events

Events are messages that represent a task to be done by the core. The programmer does not usually create or manipulate them manually, but must provide a pool of events so other core modules can communicate with each other.

### Pools

The pools module is a container for the system internal object pools. It contains pools for events and linked list nodes used by other core modules.

To configure the size of each pool created, edit *system/pools.h*.

#### Pools usage

```c
#include "system/pools.h"

// ...

pools_t pools;
pools_init(&pools);
// This allocates two pools:
//   1) pools.event_pool
//   2_ pools.llist_node_pool
```

### Scheduler

The scheduler is a module that keeps track of current execution time and keeps track of closures to be run in the future. It provides similar functionality to the `setTimeout` and `setInterval` Javascript functions.

Two queues lead in and out of it: the inbound reschedule_queue is externally fed events that have already been processed but should be rescheduled (*i.e.*: repeating timers); the outbound event_queue hold events that are due to be collected and processed. Both queues must be provided during initialisation.

This module needs access to system's event and linked list node pools.

#### Basic scheduler initialisation

```c
#include <stdlib.h>
#include "system/scheduler.h"
#include "system/pools.h"
#include "utils/circular-queue.h"

// ...

// Create necessary pools
pools_t pools;
pools_init(&pools);

// Create event queue
void *event_queue_buffer[8];
cqueue_t event_queue;
cqueue_init(&event_queue, event_queue_buffer, 3);

// Create reschedule queue
void *reschedule_queue_buffer[8];
cqueue_t reschedule_queue;
cqueue_init(&reschedule_queue, reschedule_queue_buffer, 3);

// Push it all into the scheduler
scheduler_t scheduler;
sch_init(
    &scheduler,
    &pools.llist_node_pool,
    &pools.event_pool,
    &event_queue,
    &reschedule_queue
);
```

#### Scheduler operation

The scheduler module accepts input of closures and scheduling info an then turns it into a timer event. This timer is then inserted in a timer list, which is sorted by each timer's due time.

```c
#include <stdio.h>
#include <stdint.h>
#include "utils/closure.h"

static void *print_num(closure_t *closure){
  uintptr_t num = (uintptr_t)closure->context;
  printf("%d\n", num);

  return NULL;
}

// ...

closure_t  print_one = closure_create(&print_num, (void *)1, NULL);
closure_t  print_one = closure_create(&print_num, (void *)1, NULL);
closure_t  print_three = closure_create(&print_num, (void *)3, NULL);

// Schedules to run 1000ms in the future.
sch_run_later(&scheduler, 1000, print_one);

// Schedules to run at intervals of 500ms, runs the first time after 500ms
sch_run_at_intervals(&scheduler, 500, false, print_two);

// Schedules to run at intervals of 300ms, runs the first time the next runloop
sch_run_at_intervals(&scheduler, 300, true, print_three);

```

The scheduler must be fed regularly to work. It needs both an update on the running time as an stimulus to process enqueued timers. Ideally, a hardware timer will be consistently incrementing a counter and in the main loop this counter is fed to the scheduler.

```c
uint32_t counter = 0;

void my_timer_isr(){
  isr_flag = 0;
  counter++;
}

// ...

// On the main loop
// (tip: don't do this, more on the subject later)

sch_update_timer(&scheduler, counter);
sch_manage_timers(&scheduler);
```

When the function `sch_manage_timers` is called, it iterates over the enqueued timer list and breaks it when it finds a timer scheduled further in the future. It then moves each timer to the event queue, where it will be further collected and processed.

# TODO: finish README.md
