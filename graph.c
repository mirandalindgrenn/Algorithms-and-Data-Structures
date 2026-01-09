#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

void vec_init(IntVec *v)
{
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
}

void vec_free(IntVec *v)
{
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
}

void vec_push(IntVec *v, int x)
{
    if (v->size == v->cap)
    {
        int newcap = (v->cap == 0) ? 4 : v->cap * 2;
        int *nd = (int *)realloc(v->data, (size_t)newcap * sizeof(int));
        if (!nd)
        {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        v->data = nd;
        v->cap = newcap;
    }
    v->data[v->size++] = x;
}

Graph graph_create(int n)
{
    Graph g;
    g.n = n;
    g.adj = (IntVec *)malloc((size_t)n * sizeof(IntVec));
    g.indeg = (int *)calloc((size_t)n, sizeof(int));
    if (!g.adj || !g.indeg)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i++)
        vec_init(&g.adj[i]);
    return g;
}

void graph_free(Graph *g)
{
    if (!g)
        return;
    for (int i = 0; i < g->n; i++)
        vec_free(&g->adj[i]);
    free(g->adj);
    g->adj = NULL;
    free(g->indeg);
    g->indeg = NULL;
    g->n = 0;
}

void graph_add_edge(Graph *g, int u, int v)
{
    // edge u -> v means u must finish before v can start
    vec_push(&g->adj[u], v);
    g->indeg[v] += 1;
}
