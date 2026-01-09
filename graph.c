#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

void vec_init(Vec *v)
{
    v->a = NULL;
    v->n = 0;
    v->cap = 0;
}

void vec_free(Vec *v)
{
    free(v->a);
    v->a = NULL;
    v->n = 0;
    v->cap = 0;
}

void vec_push(Vec *v, int x)
{
    if (v->n == v->cap)
    {
        int newcap = (v->cap == 0) ? 4 : 2 * v->cap;
        int *na = realloc(v->a, sizeof(int) * newcap);
        if (!na)
        {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        v->a = na;
        v->cap = newcap;
    }
    v->a[v->n++] = x;
}

Graph graph_create(int n)
{
    Graph g;
    g.n = n;
    g.succ = malloc(sizeof(Vec) * n);
    g.indeg = calloc(n, sizeof(int));
    if (!g.succ || !g.indeg)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    for (int i = 0; i < n; i++)
        vec_init(&g.succ[i]);
    return g;
}

void graph_free(Graph *g)
{
    for (int i = 0; i < g->n; i++)
        vec_free(&g->succ[i]);
    free(g->succ);
    free(g->indeg);
    g->succ = NULL;
    g->indeg = NULL;
    g->n = 0;
}

void graph_add_edge(Graph *g, int u, int v)
{
    vec_push(&g->succ[u], v);
    g->indeg[v]++;
}
