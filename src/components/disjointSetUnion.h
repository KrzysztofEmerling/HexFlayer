#ifndef DISJOIN_SET_UNION_H
#define DISJOIN_SET_UNION_H

typedef struct {
    int *parent;
    int *rank;
    int size;
} DSU;

typedef struct {
    int node;
    int parent;
    int rank;
} Change;


DSU* dsuInit(int n);

int dsuFind(DSU *dsu, int x);

void dsuUnion(DSU *dsu, int a, int b);

bool dsuConnected(DSU *dsu, int a, int b);

void dsuFree(DSU *dsu);

#endif