#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "graph.h"

typedef struct
{
    int task;
    int start;
    int finish;
    int worker; // -1 for step 2
} SchedItem;

// Step 2: unlimited workers -> EST/EFT. Returns 1 if OK else 0 if cycle.
int schedule_unlimited(const Graph *g, const int *duration,
                       int *EST, int *EFT, int *makespan);

// Step 3: limited workers (m) -> list scheduling. Returns 1 if OK else 0 if cycle.
int schedule_limited(const Graph *g, const int *duration, int m,
                     SchedItem *out, int *out_n, int *makespan);

#endif
