#include <stdio.h>
#include "graph.h"
#include "topo_sort.h"
#include "schedule.h"

int main(void)
{
    // Example tasks (A..F)
    // Dependencies: A->D, B->D, C->E, D->F, E->F
    // Durations:    A=3, B=2, C=4, D=5, E=1, F=2

    const int n = 6;
    const char *names[6] = {"A", "B", "C", "D", "E", "F"};
    int duration[6] = {3, 2, 4, 5, 1, 2};

    Graph g = graph_create(n);
    graph_add_edge(&g, 0, 3); // A->D
    graph_add_edge(&g, 1, 3); // B->D
    graph_add_edge(&g, 2, 4); // C->E
    graph_add_edge(&g, 3, 5); // D->F
    graph_add_edge(&g, 4, 5); // E->F

    // Step 1: Topological sort
    int order[6];
    if (!kahn_toposort(&g, order))
    {
        printf("Cycle detected: no valid schedule.\n");
        graph_free(&g);
        return 1;
    }

    printf("Topological order: ");
    print_topo(names, order, n);
    printf("Verify topological order: %s\n\n",
           verify_topological_order(&g, order) ? "OK" : "FAIL");

    // Step 2: Unlimited workers schedule (optimal)
    SchedItem s2[6];
    int ms2 = 0;
    if (!schedule_unlimited(&g, duration, s2, &ms2))
    {
        printf("Cycle detected in step 2.\n");
        graph_free(&g);
        return 1;
    }

    printf("Step 2 (unlimited workers) schedule:\n");
    print_schedule_unlimited(names, s2, n);
    printf("Makespan: %d\n", ms2);
    printf("Verify Step 2 schedule: %s\n\n",
           verify_schedule_precedence(&g, s2, n) ? "OK" : "FAIL");

    // Step 3: Limited workers schedule (heuristic)
    const int m_workers = 2;
    SchedItem s3[6];
    int ms3 = 0;

    if (!schedule_limited_workers(&g, duration, m_workers, s3, &ms3))
    {
        printf("Error in step 3 (cycle or invalid workers).\n");
        graph_free(&g);
        return 1;
    }

    printf("Step 3 (limited workers) schedule:\n");
    print_schedule_limited(names, s3, n);
    printf("Makespan: %d\n", ms3);
    printf("Verify Step 3 schedule: %s\n",
           verify_schedule_limited(&g, s3, n, m_workers) ? "OK" : "FAIL");

    graph_free(&g);
    return 0;
}
