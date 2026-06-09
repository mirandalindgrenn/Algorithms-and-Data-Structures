#include "schedule.h"
#include "topo.h"
#include <stdio.h>
#include <stdlib.h>

/* step 2: unlimited workers */

int schedule_unlimited(const Graph *g, const int *duration,
                       int *EST, int *EFT, int *makespan)
{
    int n = g->n;
    int *order = malloc(sizeof(int) * n);
    if (!order)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    /* Topological order required */
    if (!kahn_toposort(g, order))
    {
        free(order);
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        EST[i] = 0;
        EFT[i] = 0;
    }

    /* process tasks in topological order */
    for (int i = 0; i < n; i++)
    {
        int v = order[i];
        int best = 0;

        /* find maximum EFT among predecessors */
        for (int u = 0; u < n; u++)
        {
            for (int j = 0; j < g->succ[u].n; j++)
            {
                if (g->succ[u].a[j] == v)
                {
                    if (EFT[u] > best)
                        best = EFT[u];
                }
            }
        }

        EST[v] = best;
        EFT[v] = best + duration[v];
    }

    int ms = 0;
    for (int v = 0; v < n; v++)
        if (EFT[v] > ms)
            ms = EFT[v];

    *makespan = ms;
    free(order);
    return 1;
}

/* helper: bottom-level computation bl[v] = duration[v] + max_{v->s} bl[s]*/

static int compute_bottom_level(const Graph *g,
                                const int *duration,
                                int *bl)
{
    int n = g->n;
    int *order = malloc(sizeof(int) * n);
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

    for (int v = 0; v < n; v++)
        bl[v] = duration[v];

    /* traverse in reverse order */
    for (int i = n - 1; i >= 0; i--)
    {
        int v = order[i];
        int best = 0;

        for (int j = 0; j < g->succ[v].n; j++)
        {
            int s = g->succ[v].a[j];
            if (bl[s] > best)
                best = bl[s];
        }
        bl[v] = duration[v] + best;
    }

    free(order);
    return 1;
}

/*step 3: list scheduling limited workers. Max-heap for ready tasks (priority = bl)*/

typedef struct
{
    int *task;
    int *prio;
    int n, cap;
} MaxHeap;

static void maxh_init(MaxHeap *h, int cap)
{
    h->task = malloc(sizeof(int) * cap);
    h->prio = malloc(sizeof(int) * cap);
    if (!h->task || !h->prio)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    h->n = 0;
    h->cap = cap;
}

static void maxh_free(MaxHeap *h)
{
    free(h->task);
    free(h->prio);
}

static void maxh_swap(MaxHeap *h, int i, int j)
{
    int t = h->task[i];
    h->task[i] = h->task[j];
    h->task[j] = t;
    int p = h->prio[i];
    h->prio[i] = h->prio[j];
    h->prio[j] = p;
}

static void maxh_push(MaxHeap *h, int task, int prio)
{
    int i = h->n++;
    h->task[i] = task;
    h->prio[i] = prio;

    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->prio[p] >= h->prio[i])
            break;
        maxh_swap(h, p, i);
        i = p;
    }
}

static int maxh_empty(const MaxHeap *h)
{
    return h->n == 0;
}

static int maxh_pop(MaxHeap *h)
{
    int ret = h->task[0];
    h->n--;
    h->task[0] = h->task[h->n];
    h->prio[0] = h->prio[h->n];

    int i = 0;
    while (1)
    {
        int l = 2 * i + 1, r = 2 * i + 2;
        int best = i;

        if (l < h->n && h->prio[l] > h->prio[best])
            best = l;
        if (r < h->n && h->prio[r] > h->prio[best])
            best = r;
        if (best == i)
            break;

        maxh_swap(h, i, best);
        i = best;
    }
    return ret;
}

/* min-heap for running tasks by finish time*/

typedef struct
{
    int *finish;
    int *worker;
    int *task;
    int n, cap;
} MinHeap;

static void minh_init(MinHeap *h, int cap)
{
    h->finish = malloc(sizeof(int) * cap);
    h->worker = malloc(sizeof(int) * cap);
    h->task = malloc(sizeof(int) * cap);
    if (!h->finish || !h->worker || !h->task)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    h->n = 0;
    h->cap = cap;
}

static void minh_free(MinHeap *h)
{
    free(h->finish);
    free(h->worker);
    free(h->task);
}

static void minh_swap(MinHeap *h, int i, int j)
{
    int f = h->finish[i];
    h->finish[i] = h->finish[j];
    h->finish[j] = f;
    int w = h->worker[i];
    h->worker[i] = h->worker[j];
    h->worker[j] = w;
    int t = h->task[i];
    h->task[i] = h->task[j];
    h->task[j] = t;
}

static void minh_push(MinHeap *h, int finish, int worker, int task)
{
    int i = h->n++;
    h->finish[i] = finish;
    h->worker[i] = worker;
    h->task[i] = task;

    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->finish[p] <= h->finish[i])
            break;
        minh_swap(h, p, i);
        i = p;
    }
}

static int minh_empty(const MinHeap *h)
{
    return h->n == 0;
}

static int minh_peek_finish(const MinHeap *h)
{
    return h->finish[0];
}

static void minh_pop(MinHeap *h, int *finish, int *worker, int *task)
{
    if (finish)
        *finish = h->finish[0];
    if (worker)
        *worker = h->worker[0];
    if (task)
        *task = h->task[0];

    h->n--;
    h->finish[0] = h->finish[h->n];
    h->worker[0] = h->worker[h->n];
    h->task[0] = h->task[h->n];

    int i = 0;
    while (1)
    {
        int l = 2 * i + 1, r = 2 * i + 2;
        int best = i;

        if (l < h->n && h->finish[l] < h->finish[best])
            best = l;
        if (r < h->n && h->finish[r] < h->finish[best])
            best = r;
        if (best == i)
            break;

        minh_swap(h, i, best);
        i = best;
    }
}

/* limited workers scheduling */

int schedule_limited(const Graph *g, const int *duration, int m,
                     SchedItem *out, int *out_n, int *makespan)
{
    int n = g->n;
    if (m <= 0)
        return 0;

    int *bl = malloc(sizeof(int) * n);
    int *remain = malloc(sizeof(int) * n);
    if (!bl || !remain)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    if (!compute_bottom_level(g, duration, bl))
    {
        free(bl);
        free(remain);
        return 0;
    }

    for (int i = 0; i < n; i++)
        remain[i] = g->indeg[i];

    MaxHeap ready;
    maxh_init(&ready, n);
    for (int i = 0; i < n; i++)
        if (remain[i] == 0)
            maxh_push(&ready, i, bl[i]);

    int *freeW = malloc(sizeof(int) * m);
    if (!freeW)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    int freeCount = m;
    for (int w = 0; w < m; w++)
        freeW[w] = w;

    MinHeap running;
    minh_init(&running, n);

    int time = 0;
    int k = 0;

    while (!minh_empty(&running) || !maxh_empty(&ready))
    {
        while (freeCount > 0 && !maxh_empty(&ready))
        {
            int t = maxh_pop(&ready);
            int w = freeW[--freeCount];
            int start = time;
            int finish = start + duration[t];
            out[k++] = (SchedItem){t, start, finish, w};
            minh_push(&running, finish, w, t);
        }

        time = minh_peek_finish(&running);

        while (!minh_empty(&running) && minh_peek_finish(&running) == time)
        {
            int fin, w, t;
            minh_pop(&running, &fin, &w, &t);
            freeW[freeCount++] = w;

            for (int j = 0; j < g->succ[t].n; j++)
            {
                int s = g->succ[t].a[j];
                if (--remain[s] == 0)
                    maxh_push(&ready, s, bl[s]);
            }
        }
    }

    int ms = 0;
    for (int i = 0; i < k; i++)
        if (out[i].finish > ms)
            ms = out[i].finish;

    *makespan = ms;
    *out_n = k;

    minh_free(&running);
    maxh_free(&ready);
    free(freeW);
    free(bl);
    free(remain);
    return 1;
}
