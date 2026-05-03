#include "dsu.hpp"
#include <cstdlib>

DSU::DSU(int n) {
    parent = (int*)malloc(n * sizeof(int));
    rank   = (int*)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        parent[i] = i;
        rank[i] = 0;
    }
}

DSU::~DSU() {
    free(parent);
    free(rank);
}

int DSU::find(int x) {
    while (parent[x] != x)
        x = parent[x];
    return x;
}
int DSU::find_compress(int x) {
    if (parent[x] != x) {
        parent[x] = find_compress(parent[x]);
    }
    return parent[x];
}

void DSU::save(int node, DSUChange* changes, int& changes_size) {
    DSUChange c;
    c.node = node;
    c.parent = parent[node];
    c.rank = rank[node];

    changes[changes_size++] = c;
}

void DSU::unite(int a, int b, DSUChange* changes, int& changes_size) {

    int rootA = find(a);
    int rootB = find(b);

    if (rootA == rootB)
        return;

    if (rank[rootA] < rank[rootB]) {
        int tmp = rootA;
        rootA = rootB;
        rootB = tmp;
    }

    save(rootB, changes, changes_size);
    parent[rootB] = rootA;

    if (rank[rootA] == rank[rootB]) {
        save(rootA, changes, changes_size);
        rank[rootA]++;
    }
}
void DSU::unite_compress(int a, int b) {
    int rootA = find_compress(a);
    int rootB = find_compress(b);

    if (rootA == rootB)
        return;

    if (rank[rootA] < rank[rootB]) {
        int tmp = rootA;
        rootA = rootB;
        rootB = tmp;
    }

    parent[rootB] = rootA;

    if (rank[rootA] == rank[rootB]) {
        rank[rootA]++;
    }
}


bool DSU::connected(int a, int b) {
    return find(a) == find(b);
}

int DSU::snapshot(int changes_size) {
    return changes_size;
}

void DSU::rollback(DSUChange* changes,
                   int& changes_size,
                   int snapshot) {

    while (changes_size > snapshot) {
        DSUChange c = changes[--changes_size];
        parent[c.node] = c.parent;
        rank[c.node]   = c.rank;
    }
}