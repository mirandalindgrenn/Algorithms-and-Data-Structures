#include <stdio.h>

#define MAXN 10  // max antal noder i grafen

// Kahn's algorithm för topologisk sortering
void topo_sort_kahn(int n, int adj[MAXN][MAXN], int result[]) {
    int indegree[MAXN] = {0};
    int queue[MAXN];
    int front = 0, back = 0;

    // 1. Beräkna indegree för alla noder
    for (int j = 0; j < n; j++) {
        indegree[j] = 0;
        for (int i = 0; i < n; i++) {
            if (adj[i][j] == 1) {
                indegree[j]++;
            }
        }
    }

    // 2. Lägg alla noder med indegree = 0 i kön
    for (int i = 0; i < n; i++) {
        if (indegree[i] == 0) {
            queue[back++] = i;
        }
    }

    int idx = 0; // index i result-arrayen

    // 3. Processa kön
    while (front < back) {
        int u = queue[front++];   // ta ut första i kön
        result[idx++] = u;        // lägg i resultatordningen

        // 4. "Ta bort" kanten u -> v för alla grannar v
        for (int v = 0; v < n; v++) {
            if (adj[u][v] == 1) {
                indegree[v]--;
                if (indegree[v] == 0) {
                    queue[back++] = v;
                }
            }
        }
    }

    // 5. Kolla om vi verkligen fick med alla noder (annars fanns en cykel)
    if (idx != n) {
        printf("Grafen har en cykel eller är inte en DAG!\n");
    }
}

int main(void) {
    int n = 4; // antal tasks

    // Namn på tasks (bara för utskrift)
    const char *names[] = {"A", "B", "C", "D"};

    // Adjacency matrix adj[i][j] = 1 om det finns en kant i->j (i måste före j)
    int adj[MAXN][MAXN] = {0};

    // A = 0, B = 1, C = 2, D = 3
    // B beror på A: A -> B  =>  edge 0 -> 1
    adj[0][1] = 1;
    // C beror på A: A -> C  =>  edge 0 -> 2
    adj[0][2] = 1;
    // D beror på B och C: B -> D, C -> D
    adj[1][3] = 1;
    adj[2][3] = 1;

    int result[MAXN];

    topo_sort_kahn(n, adj, result);

    printf("Topologisk ordning:\n");
    for (int i = 0; i < n; i++) {
        printf("%s ", names[result[i]]);
    }
    printf("\n");

    return 0;
}
