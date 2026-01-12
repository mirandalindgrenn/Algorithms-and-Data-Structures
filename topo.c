#include "topo.h"
#include <stdio.h>
#include <stdlib.h>

int kahn_toposort(const Graph *g, int *order)
{
    printf("[DEBUG topo] g=%p g->n=%d g->succ=%p g->indeg=%p\n",
           (void *)g, g->n, (void *)g->succ, (void *)g->indeg);
    fflush(stdout);

    int n = g->n;

    int *indeg = malloc(sizeof(int) * n);
    if (!indeg)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i++)
        indeg[i] = g->indeg[i];

    int *q = malloc(sizeof(int) * n);
    if (!q)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    int front = 0, back = 0;

    for (int i = 0; i < n; i++)
        if (indeg[i] == 0)
            q[back++] = i;

    int k = 0;

    while (front < back)
    {
        int u = q[front++];
        order[k++] = u;

        for (int j = 0; j < g->succ[u].n; j++)
        {
            int v = g->succ[u].a[j];
            indeg[v]--;
            if (indeg[v] == 0)
                q[back++] = v;
        }
    }

    free(q);
    free(indeg);
    return (k == n) ? 1 : 0;
}
