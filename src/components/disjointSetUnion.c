#include "disjointSetUnion.h"

typedef struct {
    int *parent;
    int *rank;
    int size;
} DSU;

DSU* dsuInit(int n) {
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

int dsuFind(DSU *dsu, int x) {
    if (dsu->parent[x] != x)
        dsu->parent[x] = dsu_find(dsu, dsu->parent[x]);

    return dsu->parent[x];
}

void dsuUnion(DSU *dsu, int a, int b) {
    int rootA = dsu_find(dsu, a);
    int rootB = dsu_find(dsu, b);

    if (rootA == rootB)
        return;

    if (dsu->rank[rootA] < dsu->rank[rootB]) {
        dsu->parent[rootA] = rootB;
    } 
    else if (dsu->rank[rootA] > dsu->rank[rootB]) {
        dsu->parent[rootB] = rootA;
    } 
    else {
        dsu->parent[rootB] = rootA;
        dsu->rank[rootA]++;
    }
}

bool dsuConnected(DSU *dsu, int a, int b) {
    return dsuFind(dsu, a) == dsu_find(dsu, b);
}

void dsuFree(DSU *dsu) {
    free(dsu->parent);
    free(dsu->rank);
    free(dsu);
}
