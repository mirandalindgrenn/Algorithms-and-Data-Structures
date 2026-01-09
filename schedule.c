#include "schedule.h"
#include "topo_sort.h"
#include <stdio.h>
#include <stdlib.h>

/* ---------------- Step 2: Unlimited workers schedule ---------------- */

int schedule_unlimited(const Graph *g,
                       const int *duration,
                       SchedItem *items_out,
                       int *makespan_out)
{
    int n = g->n;

    int *order = (int *)malloc((size_t)n * sizeof(int));
    if (!order)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    if (!kahn_toposort(g, order))
    {
        free(order);
        return 0;
    }

    // Build predecessor lists
    IntVec *pred = (IntVec *)malloc((size_t)n * sizeof(IntVec));
    if (!pred)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    for (int i = 0; i < n; i++)
        vec_init(&pred[i]);

    for (int u = 0; u < n; u++)
    {
        for (int j = 0; j < g->adj[u].size; j++)
        {
            int v = g->adj[u].data[j];
            vec_push(&pred[v], u);
        }
    }

    int *est = (int *)calloc((size_t)n, sizeof(int));
    int *eft = (int *)calloc((size_t)n, sizeof(int));
    if (!est || !eft)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i++)
    {
        int v = order[i];
        int start = 0;
        for (int j = 0; j < pred[v].size; j++)
        {
            int p = pred[v].data[j];
            if (eft[p] > start)
                start = eft[p];
        }
        est[v] = start;
        eft[v] = start + duration[v];

        items_out[i] = (SchedItem){.task = v, .start = est[v], .finish = eft[v], .worker = -1};
    }

    int ms = 0;
    for (int v = 0; v < n; v++)
        if (eft[v] > ms)
            ms = eft[v];
    *makespan_out = ms;

    for (int i = 0; i < n; i++)
        vec_free(&pred[i]);
    free(pred);
    free(est);
    free(eft);
    free(order);
    return 1;
}

int verify_schedule_precedence(const Graph *g, const SchedItem *items, int n)
{
    int *start = (int *)malloc((size_t)n * sizeof(int));
    int *finish = (int *)malloc((size_t)n * sizeof(int));
    if (!start || !finish)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    for (int i = 0; i < n; i++)
    {
        start[items[i].task] = items[i].start;
        finish[items[i].task] = items[i].finish;
    }

    for (int u = 0; u < n; u++)
    {
        for (int j = 0; j < g->adj[u].size; j++)
        {
            int v = g->adj[u].data[j];
            if (finish[u] > start[v])
            {
                free(start);
                free(finish);
                return 0;
            }
        }
    }

    free(start);
    free(finish);
    return 1;
}

/* ---------------- Step 3: Limited workers schedule (heuristic) ---------------- */

/*
  bottom-level:
    bl[v] = duration[v] + max_{v->s} bl[s], or duration[v] if no succ.
*/
static int compute_bottom_level(const Graph *g, const int *duration, int *bl_out)
{
    int n = g->n;
    int *order = (int *)malloc((size_t)n * sizeof(int));
    if (!order)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    if (!kahn_toposort(g, order))
    {
        free(order);
        return 0;
    }

    for (int i = 0; i < n; i++)
        bl_out[i] = duration[i];

    for (int i = n - 1; i >= 0; i--)
    {
        int v = order[i];
        int best = 0;
        for (int j = 0; j < g->adj[v].size; j++)
        {
            int s = g->adj[v].data[j];
            if (bl_out[s] > best)
                best = bl_out[s];
        }
        bl_out[v] = duration[v] + best;
    }

    free(order);
    return 1;
}

/* Max-heap for ready tasks by priority (bottom-level). */
typedef struct
{
    int *task;
    int *prio;
    int size;
    int cap;
} MaxHeap;

static void maxheap_init(MaxHeap *h, int cap)
{
    h->task = (int *)malloc((size_t)cap * sizeof(int));
    h->prio = (int *)malloc((size_t)cap * sizeof(int));
    if (!h->task || !h->prio)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    h->size = 0;
    h->cap = cap;
}
static void maxheap_free(MaxHeap *h)
{
    free(h->task);
    free(h->prio);
    h->task = NULL;
    h->prio = NULL;
    h->size = h->cap = 0;
}
static void maxheap_swap(MaxHeap *h, int a, int b)
{
    int tt = h->task[a];
    h->task[a] = h->task[b];
    h->task[b] = tt;
    int pp = h->prio[a];
    h->prio[a] = h->prio[b];
    h->prio[b] = pp;
}
static void maxheap_push(MaxHeap *h, int task, int prio)
{
    int i = h->size++;
    h->task[i] = task;
    h->prio[i] = prio;
    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->prio[p] >= h->prio[i])
            break;
        maxheap_swap(h, p, i);
        i = p;
    }
}
static int maxheap_empty(const MaxHeap *h) { return h->size == 0; }
static int maxheap_pop(MaxHeap *h, int *prio_out)
{
    int task = h->task[0];
    if (prio_out)
        *prio_out = h->prio[0];

    h->size--;
    h->task[0] = h->task[h->size];
    h->prio[0] = h->prio[h->size];

    int i = 0;
    while (1)
    {
        int l = 2 * i + 1, r = 2 * i + 2;
        int largest = i;
        if (l < h->size && h->prio[l] > h->prio[largest])
            largest = l;
        if (r < h->size && h->prio[r] > h->prio[largest])
            largest = r;
        if (largest == i)
            break;
        maxheap_swap(h, i, largest);
        i = largest;
    }
    return task;
}

/* Min-heap for ongoing tasks by finish time. */
typedef struct
{
    int *finish;
    int *worker;
    int *task;
    int size;
    int cap;
} MinHeap;

static void minheap_init(MinHeap *h, int cap)
{
    h->finish = (int *)malloc((size_t)cap * sizeof(int));
    h->worker = (int *)malloc((size_t)cap * sizeof(int));
    h->task = (int *)malloc((size_t)cap * sizeof(int));
    if (!h->finish || !h->worker || !h->task)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    h->size = 0;
    h->cap = cap;
}
static void minheap_free(MinHeap *h)
{
    free(h->finish);
    free(h->worker);
    free(h->task);
    h->finish = h->worker = h->task = NULL;
    h->size = h->cap = 0;
}
static void minheap_swap(MinHeap *h, int a, int b)
{
    int tf = h->finish[a];
    h->finish[a] = h->finish[b];
    h->finish[b] = tf;
    int tw = h->worker[a];
    h->worker[a] = h->worker[b];
    h->worker[b] = tw;
    int tt = h->task[a];
    h->task[a] = h->task[b];
    h->task[b] = tt;
}
static void minheap_push(MinHeap *h, int finish, int worker, int task)
{
    int i = h->size++;
    h->finish[i] = finish;
    h->worker[i] = worker;
    h->task[i] = task;

    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->finish[p] <= h->finish[i])
            break;
        minheap_swap(h, p, i);
        i = p;
    }
}
static int minheap_empty(const MinHeap *h) { return h->size == 0; }
static int minheap_peek_finish(const MinHeap *h) { return h->finish[0]; }
static void minheap_pop(MinHeap *h, int *finish, int *worker, int *task)
{
    if (finish)
        *finish = h->finish[0];
    if (worker)
        *worker = h->worker[0];
    if (task)
        *task = h->task[0];

    h->size--;
    h->finish[0] = h->finish[h->size];
    h->worker[0] = h->worker[h->size];
    h->task[0] = h->task[h->size];

    int i = 0;
    while (1)
    {
        int l = 2 * i + 1, r = 2 * i + 2;
        int smallest = i;
        if (l < h->size && h->finish[l] < h->finish[smallest])
            smallest = l;
        if (r < h->size && h->finish[r] < h->finish[smallest])
            smallest = r;
        if (smallest == i)
            break;
        minheap_swap(h, i, smallest);
        i = smallest;
    }
}

int schedule_limited_workers(const Graph *g,
                             const int *duration,
                             int m_workers,
                             SchedItem *items_out,
                             int *makespan_out)
{
    int n = g->n;
    if (m_workers <= 0)
        return 0;

    int *bl = (int *)malloc((size_t)n * sizeof(int));
    if (!bl)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    if (!compute_bottom_level(g, duration, bl))
    {
        free(bl);
        return 0;
    }

    int *remain = (int *)malloc((size_t)n * sizeof(int));
    if (!remain)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    for (int i = 0; i < n; i++)
        remain[i] = g->indeg[i];

    MaxHeap ready;
    maxheap_init(&ready, n);
    for (int i = 0; i < n; i++)
        if (remain[i] == 0)
            maxheap_push(&ready, i, bl[i]);

    int *free_workers = (int *)malloc((size_t)m_workers * sizeof(int));
    if (!free_workers)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    int free_count = m_workers;
    for (int w = 0; w < m_workers; w++)
        free_workers[w] = w;

    MinHeap ongoing;
    minheap_init(&ongoing, n);

    int time = 0;
    int out_k = 0;

    // start tasks at time 0
    while (free_count > 0 && !maxheap_empty(&ready))
    {
        int pr;
        int t = maxheap_pop(&ready, &pr);
        int w = free_workers[--free_count];
        int start = time;
        int finish = start + duration[t];

        items_out[out_k++] = (SchedItem){.task = t, .start = start, .finish = finish, .worker = w};
        minheap_push(&ongoing, finish, w, t);
    }

    while (!minheap_empty(&ongoing) || !maxheap_empty(&ready))
    {
        if (minheap_empty(&ongoing))
        {
            // nothing running; start ready tasks at current time
            while (free_count > 0 && !maxheap_empty(&ready))
            {
                int pr;
                int t = maxheap_pop(&ready, &pr);
                int w = free_workers[--free_count];
                int start = time;
                int finish = start + duration[t];

                items_out[out_k++] = (SchedItem){.task = t, .start = start, .finish = finish, .worker = w};
                minheap_push(&ongoing, finish, w, t);
            }
            continue;
        }

        // jump to next finish
        time = minheap_peek_finish(&ongoing);

        // pop all tasks finishing now
        while (!minheap_empty(&ongoing) && minheap_peek_finish(&ongoing) == time)
        {
            int fin, w, t;
            minheap_pop(&ongoing, &fin, &w, &t);
            free_workers[free_count++] = w;

            // release successors
            for (int j = 0; j < g->adj[t].size; j++)
            {
                int s = g->adj[t].data[j];
                remain[s]--;
                if (remain[s] == 0)
                    maxheap_push(&ready, s, bl[s]);
            }
        }

        // start tasks at same time
        while (free_count > 0 && !maxheap_empty(&ready))
        {
            int pr;
            int t = maxheap_pop(&ready, &pr);
            int w = free_workers[--free_count];
            int start = time;
            int finish = start + duration[t];

            items_out[out_k++] = (SchedItem){.task = t, .start = start, .finish = finish, .worker = w};
            minheap_push(&ongoing, finish, w, t);
        }
    }

    int ms = 0;
    for (int i = 0; i < out_k; i++)
        if (items_out[i].finish > ms)
            ms = items_out[i].finish;
    *makespan_out = ms;

    minheap_free(&ongoing);
    free(free_workers);
    maxheap_free(&ready);
    free(remain);
    free(bl);

    return 1;
}

int verify_schedule_limited(const Graph *g, const SchedItem *items, int n, int m_workers)
{
    if (!verify_schedule_precedence(g, items, n))
        return 0;

    // overlap check per worker (O(n^2), ok for coursework)
    for (int w = 0; w < m_workers; w++)
    {
        for (int i = 0; i < n; i++)
        {
            if (items[i].worker != w)
                continue;
            for (int j = i + 1; j < n; j++)
            {
                if (items[j].worker != w)
                    continue;
                int aS = items[i].start, aF = items[i].finish;
                int bS = items[j].start, bF = items[j].finish;
                if (!(aF <= bS || bF <= aS))
                    return 0;
            }
        }
    }

    for (int i = 0; i < n; i++)
    {
        if (items[i].worker < 0 || items[i].worker >= m_workers)
            return 0;
    }
    return 1;
}

/* ---------------- Pretty-print helpers ---------------- */

void print_topo(const char **names, const int *order, int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("%s%s", names[order[i]], (i + 1 == n) ? "" : " -> ");
    }
    printf("\n");
}

void print_schedule_unlimited(const char **names, const SchedItem *items, int n)
{
    printf("Task  Start  Finish\n");
    for (int i = 0; i < n; i++)
    {
        printf("%-5s %-6d %-6d\n", names[items[i].task], items[i].start, items[i].finish);
    }
}

void print_schedule_limited(const char **names, const SchedItem *items, int n)
{
    printf("Task  Worker  Start  Finish\n");
    for (int i = 0; i < n; i++)
    {
        printf("%-5s %-7d %-6d %-6d\n", names[items[i].task], items[i].worker, items[i].start, items[i].finish);
    }
}
