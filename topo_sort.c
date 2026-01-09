#include "topo_sort.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int *data;
    int head, tail, cap;
} IntQueue;

static void q_init(IntQueue *q, int cap)
{
    q->data = (int *)malloc((size_t)cap * sizeof(int));
    if (!q->data)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    q->head = q->tail = 0;
    q->cap = cap;
}
static void q_free(IntQueue *q)
{
    free(q->data);
    q->data = NULL;
    q->head = q->tail = q->cap = 0;
}
static int q_empty(const IntQueue *q) { return q->head == q->tail; }
static void q_push(IntQueue *q, int x) { q->data[q->tail++] = x; }
static int q_pop(IntQueue *q) { return q->data[q->head++]; }

int kahn_toposort(const Graph *g, int *order_out)
{
    int n = g->n;

    int *indeg = (int *)malloc((size_t)n * sizeof(int));
    if (!indeg)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    for (int i = 0; i < n; i++)
        indeg[i] = g->indeg[i];

    IntQueue q;
    q_init(&q, n);
    for (int i = 0; i < n; i++)
    {
        if (indeg[i] == 0)
            q_push(&q, i);
    }

    int k = 0;
    while (!q_empty(&q))
    {
        int u = q_pop(&q);
        order_out[k++] = u;

        for (int j = 0; j < g->adj[u].size; j++)
        {
            int v = g->adj[u].data[j];
            indeg[v]--;
            if (indeg[v] == 0)
                q_push(&q, v);
        }
    }

    q_free(&q);
    free(indeg);

    return (k == n) ? 1 : 0;
}

int verify_topological_order(const Graph *g, const int *order)
{
    int n = g->n;
    int *pos = (int *)malloc((size_t)n * sizeof(int));
    if (!pos)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i++)
        pos[order[i]] = i;

    for (int u = 0; u < n; u++)
    {
        for (int j = 0; j < g->adj[u].size; j++)
        {
            int v = g->adj[u].data[j];
            if (pos[u] >= pos[v])
            {
                free(pos);
                return 0;
            }
        }
    }

    free(pos);
    return 1;
}
