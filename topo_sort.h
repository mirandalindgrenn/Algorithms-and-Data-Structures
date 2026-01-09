#ifndef TOPO_SORT_H
#define TOPO_SORT_H

#include "graph.h"

// Returns 1 on success (fills order_out[n]), 0 if cycle exists.
int kahn_toposort(const Graph *g, int *order_out);

// Verify: for every edge u->v we must have pos[u] < pos[v]
int verify_topological_order(const Graph *g, const int *order);

#endif
