#ifndef TOPO_H
#define TOPO_H

#include "graph.h"

// Returns 1 if DAG and fills order[0..n-1], else 0 if cycle.
int kahn_toposort(const Graph *g, int *order);

#endif
