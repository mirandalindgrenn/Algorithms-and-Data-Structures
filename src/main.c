#include <stdio.h>
#include "graph.h"
#include "topo.h"
#include "schedule.h"

int main(void)
{
    int make_cycle = 0;
    // Example: tasks A..F => 0..5
    int n = 6;
    const char *name[] = {"A", "B", "C", "D", "E", "F"};

    // durations
    int duration[] = {3, 2, 4, 5, 1, 2};

    Graph g = graph_create(n);

    printf("[DEBUG main] n=%d\n", n);
    printf("[DEBUG main] g.n=%d g.succ=%p g.indeg=%p\n", g.n, (void *)g.succ, (void *)g.indeg);
    fflush(stdout);
    // edges (prerequisites)
    graph_add_edge(&g, 0, 3); // A -> D
    graph_add_edge(&g, 1, 3); // B -> D
    graph_add_edge(&g, 2, 4); // C -> E
    graph_add_edge(&g, 3, 5); // D -> F
    graph_add_edge(&g, 4, 5); // E -> F

    if (make_cycle)
    {
        graph_add_edge(&g, 5, 0); // F -> A
    }

    // Step 1: topological order
    int order[6];
    if (!kahn_toposort(&g, order))
    {
        printf("Cycle detected (not a DAG)\n");
        graph_free(&g);
        return 1;
    }
    printf("Step 1 (topological order): ");
    for (int i = 0; i < n; i++)
    {
        printf("%s%s", name[order[i]], (i + 1 == n) ? "\n" : " ");
    }
    printf("\n");

    // Step 2: unlimited workers (EST/EFT)
    int EST[6], EFT[6], ms2;
    if (!schedule_unlimited(&g, duration, EST, EFT, &ms2))
    {
        printf("Cycle detected in step 2\n");
        graph_free(&g);
        return 1;
    }
    printf("Step 2 (unlimited workers):\n");
    printf("Task  EST  EFT\n");
    for (int i = 0; i < n; i++)
    {
        printf("%-5s %-4d %-4d\n", name[i], EST[i], EFT[i]);
    }
    printf("Makespan: %d\n\n", ms2);

    // Step 3: limited workers (list scheduling heuristic)
    int m = 2;
    SchedItem out[6];
    int out_n = 0, ms3 = 0;
    if (!schedule_limited(&g, duration, m, out, &out_n, &ms3))
    {
        printf("Error in step 3\n");
        graph_free(&g);
        return 1;
    }
    printf("Step 3 (m=%d workers, heuristic):\n", m);
    printf("Task  Worker  Start  Finish\n");
    for (int i = 0; i < out_n; i++)
    {
        printf("%-5s %-7d %-6d %-6d\n",
               name[out[i].task], out[i].worker, out[i].start, out[i].finish);
    }
    printf("Makespan: %d\n", ms3);

    graph_free(&g);
    return 0;
}
