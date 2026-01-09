#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>

typedef struct
{
    int *data;
    int size;
    int cap;
} IntVec;

void vec_init(IntVec *v);
void vec_free(IntVec *v);
void vec_push(IntVec *v, int x);

typedef struct
{
    int n;       // number of tasks
    IntVec *adj; // adj[u] = successors v
    int *indeg;  // indeg[v] = number of prerequisites
} Graph;

Graph graph_create(int n);
void graph_free(Graph *g);
void graph_add_edge(Graph *g, int u, int v); // u -> v

#endif
