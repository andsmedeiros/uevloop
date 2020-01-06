/* 
 * File:   container.h
 * Author: kazeshi
 *
 * Created on January 4, 2020, 5:30 PM
 */

#ifndef CONTAINER_H
#define	CONTAINER_H

#include "pools.h"
#include "scheduler.h"

typedef struct container container_t;
struct container{
    pools_t pools;
    scheduler_t scheduler;
};

#endif	/* CONTAINER_H */

