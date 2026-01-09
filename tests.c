#include <stdio.h>
#include <assert.h>
#include "graph.h"
#include "topo.h"
#include "schedule.h"

// helper to check precedence using EST/EFT (step 2)
static void assert_precedence_unlimited(const Graph *g, const int *EST, const int *EFT)
{
    int n = g->n;
    for (int u = 0; u < n; u++)
    {
        for (int j = 0; j < g->succ[u].n; j++)
        {
            int v = g->succ[u].a[j];
            // u must finish before v starts
            assert(EFT[u] <= EST[v]);
        }
    }
}

static void test_topo_and_step2_example(void)
{
    int n = 6;
    int duration[] = {3, 2, 4, 5, 1, 2};

    Graph g = graph_create(n);
    graph_add_edge(&g, 0, 3); // A->D
    graph_add_edge(&g, 1, 3); // B->D
    graph_add_edge(&g, 2, 4); // C->E
    graph_add_edge(&g, 3, 5); // D->F
    graph_add_edge(&g, 4, 5); // E->F

    // Step 1
    int order[6];
    assert(kahn_toposort(&g, order) == 1);

    // Step 2
    int EST[6], EFT[6], ms2 = 0;
    assert(schedule_unlimited(&g, duration, EST, EFT, &ms2) == 1);
    assert_precedence_unlimited(&g, EST, EFT);

    // for this example, makespan should be 10 (critical path A->D->F is 3+5+2)
    assert(ms2 == 10);

    graph_free(&g);
}

static void test_cycle_detection(void)
{
    // 0->1, 1->2, 2->0 cycle
    Graph g = graph_create(3);
    graph_add_edge(&g, 0, 1);
    graph_add_edge(&g, 1, 2);
    graph_add_edge(&g, 2, 0);

    int order[3];
    assert(kahn_toposort(&g, order) == 0);

    graph_free(&g);
}

static void test_step3_runs(void)
{
    // Just check it produces a schedule with all tasks
    int n = 4;
    int duration[] = {2, 1, 3, 2};

    // edges: 0->2, 1->2, 2->3
    Graph g = graph_create(n);
    graph_add_edge(&g, 0, 2);
    graph_add_edge(&g, 1, 2);
    graph_add_edge(&g, 2, 3);

    SchedItem out[4];
    int out_n = 0, ms3 = 0;
    assert(schedule_limited(&g, duration, 2, out, &out_n, &ms3) == 1);
    assert(out_n == n);

    graph_free(&g);
}

int main(void)
{
    test_topo_and_step2_example();
    test_cycle_detection();
    test_step3_runs();

    printf("All tests passed.\n");
    return 0;
}
