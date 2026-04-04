#ifndef DSU_H
#define DSU_H

typedef struct {
    int node;
    int parent;
    int rank;
} DSUChange;

typedef struct {
    int *parent;
    int *rank;
    int size;
} DSU;

DSU* dsu_create(int n);
void dsu_free(DSU *dsu);

int dsu_find(DSU *dsu, int x);

void dsu_union(DSU *dsu, int a, int b,
               DSUChange *changes, int *changes_size);

int dsu_connected(DSU *dsu, int a, int b);

int dsu_snapshot(int changes_size);

void dsu_rollback(DSU *dsu,
                  DSUChange *changes,
                  int *changes_size,
                  int snapshot);

#endif