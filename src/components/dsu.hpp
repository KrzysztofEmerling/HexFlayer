#pragma once

struct DSUChange {
    int node;
    int parent;
    int rank;
};

class DSU {
private:
    int* parent;
    int* rank;

    void save(int node, DSUChange* changes, int& changes_size);

public:
    DSU(int n);
    ~DSU();

    int find(int x);

    void unite(int a, int b,
               DSUChange* changes, int& changes_size);

    bool connected(int a, int b);

    static int snapshot(int changes_size);

    void rollback(DSUChange* changes,
                  int& changes_size,
                  int snapshot);
};