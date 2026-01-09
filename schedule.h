#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "graph.h"

typedef struct
{
    int task;
    int start;
    int finish;
    int worker; // -1 for unlimited case
} SchedItem;

// Step 2: unlimited workers (optimal earliest-start schedule). Returns 1 if OK, 0 if cycle.
int schedule_unlimited(const Graph *g,
                       const int *duration,
                       SchedItem *items_out,
                       int *makespan_out);

// Verify precedence: for every edge u->v, finish[u] <= start[v]
int verify_schedule_precedence(const Graph *g, const SchedItem *items, int n);

// Step 3: limited workers (heuristic list scheduling). Returns 1 if OK, 0 if cycle or m<=0.
int schedule_limited_workers(const Graph *g,
                             const int *duration,
                             int m_workers,
                             SchedItem *items_out,
                             int *makespan_out);

// Verify limited schedule: precedence + no overlap on same worker
int verify_schedule_limited(const Graph *g,
                            const SchedItem *items,
                            int n,
                            int m_workers);

// Printing helpers (för demo/presentation)
void print_topo(const char **names, const int *order, int n);
void print_schedule_unlimited(const char **names, const SchedItem *items, int n);
void print_schedule_limited(const char **names, const SchedItem *items, int n);

#endif
