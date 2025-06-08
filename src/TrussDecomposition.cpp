// TrussDecomposition.cpp
#include "TrussDecomposition.h"

// 运算符重载
inline bool operator<(Edge e1, Edge e2) {
    return (e1.first < e2.first) || (e1.first == e2.first && e1.second < e2.second);
}


void TrussDecomposition::readGraph(std::string fileName)
{
    std::ifstream f(fileName);
    if (!f.is_open()) {
        std::cerr << "Failed to open file : " << fileName << std::endl;
        exit(1);
    }

    f >> V >> E;
    deg.resize(V, 0);
    adj.resize(V);

    for (int i = 0; i < V; i++) {
        int v_idx, nums;
        f >> v_idx >> nums;
        std::vector<int> neighbors(nums);
        
        for (int j = 0; j < nums; j++) {
            f >> neighbors[j];
            int neighbor = neighbors[j];
            if (neighbor >= V) {
                std::cerr << "Error: neighbor index out of range" << std::endl;
                exit(1);
            }
            if (adj[i].find(neighbor) == adj[i].end() && i != neighbor) {
                adj[i][neighbor] = 0;
                adj[neighbor][i] = 0;
                deg[i]++;
                if (i != neighbor) deg[neighbor]++;
            }
        }
    }
}

int TrussDecomposition::getVnum()
{
    return V;
}

int TrussDecomposition::getEnum()
{
    return E;
}