#include "dsu.h"
#include <stdlib.h>

DSU* dsu_create(int n) {
    DSU *dsu = malloc(sizeof(DSU));

    dsu->size = n;
    dsu->parent = malloc(n * sizeof(int));
    dsu->rank   = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        dsu->parent[i] = i;
        dsu->rank[i] = 0;
    }

    return dsu;
}

void dsu_free(DSU *dsu) {
    free(dsu->parent);
    free(dsu->rank);
    free(dsu);
}

int dsu_find(DSU *dsu, int x) {
    while (dsu->parent[x] != x)
        x = dsu->parent[x];
    return x;
}

static void dsu_save(DSU *dsu, int node,
                     DSUChange *changes, int *changes_size) {
    DSUChange c;
    c.node = node;
    c.parent = dsu->parent[node];
    c.rank = dsu->rank[node];

    changes[(*changes_size)++] = c;
}

void dsu_union(DSU *dsu, int a, int b,
               DSUChange *changes, int *changes_size) {

    int rootA = dsu_find(dsu, a);
    int rootB = dsu_find(dsu, b);

    if (rootA == rootB)
        return;

    if (dsu->rank[rootA] < dsu->rank[rootB]) {
        int tmp = rootA;
        rootA = rootB;
        rootB = tmp;
    }

    dsu_save(dsu, rootB, changes, changes_size);
    dsu->parent[rootB] = rootA;

    if (dsu->rank[rootA] == dsu->rank[rootB]) {
        dsu_save(dsu, rootA, changes, changes_size);
        dsu->rank[rootA]++;
    }
}

int dsu_connected(DSU *dsu, int a, int b) {
    return dsu_find(dsu, a) == dsu_find(dsu, b);
}


int dsu_snapshot(int changes_size) {
    return changes_size;
}

void dsu_rollback(DSU *dsu,
                  DSUChange *changes,
                  int *changes_size,
                  int snapshot) {

    while (*changes_size > snapshot) {
        DSUChange c = changes[--(*changes_size)];
        dsu->parent[c.node] = c.parent;
        dsu->rank[c.node]   = c.rank;
    }
}