#ifndef GRAPH_H
#define GRAPH_H

typedef struct
{
    int *a;
    int n;
    int cap;
} Vec;

typedef struct
{
    int n;      // number of tasks
    Vec *succ;  // succ[u] = list of successors v
    int *indeg; // indeg[v] = number of prerequisites
} Graph;

void vec_init(Vec *v);
void vec_free(Vec *v);
void vec_push(Vec *v, int x);

Graph graph_create(int n);
void graph_free(Graph *g);
void graph_add_edge(Graph *g, int u, int v); // u -> v

#endif
