#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

using namespace std;

typedef pair<int, int> Edge;

// 为 Edge 类型定义哈希函数
struct EdgeHash {
    size_t operator()(const Edge& e) const {
        auto hash1 = hash<int>{}(e.first);
        auto hash2 = hash<int>{}(e.second);
        // 组合哈希值的一种方法
        return hash1 ^ (hash2 << 1);
    }
};

// return (adj, V, E)
pair<vector<vector<int>>, pair<int, int>> readGraph(string& filename) {
    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "Failed to open file : " << filename << endl;
        exit(1);
    }

    int V, E;
    f >> V >> E;
    vector<vector<int>> G(V);

    // 临时存储每条边的反向边
    vector<vector<int>> reverseEdges(V);

    for (int i = 0; i < V; i++) {
        int vertexId, nums;
        f >> vertexId; // 读取顶点编号
        f >> nums;     // 读取邻居数量
        vector<int> neighbors(nums);
        
        for (int j = 0; j < nums; j++) {
            f >> neighbors[j];
            // 记录反向边
            reverseEdges[neighbors[j]].push_back(vertexId);
        }
        G[i] = neighbors;
    }

    // 将反向边添加到邻接表中
    for (int u = 0; u < V; u++) {
        // 合并原始邻居和反向邻居
        vector<int> combinedNeighbors = G[u];
        combinedNeighbors.insert(combinedNeighbors.end(), reverseEdges[u].begin(), reverseEdges[u].end());
        
        // 去重、排序
        sort(combinedNeighbors.begin(), combinedNeighbors.end());
        // unique() 算法：将相邻的重复元素移动到容器末尾.返回指向第一个重复元素的迭代器（即去重后的逻辑终点）
        // erase() 方法：删除从 unique() 返回的迭代器到容器末尾的所有元素真正释放容器占用的内存空间
        combinedNeighbors.erase(unique(combinedNeighbors.begin(), combinedNeighbors.end()), combinedNeighbors.end());
        
        G[u] = combinedNeighbors;
    }

    f.close();
    // 边数需要调整为无向边的数量
    int undirectedE = 0;
    for (int u = 0; u < V; u++) {
        undirectedE += G[u].size();
    }
    undirectedE /= 2;

    return make_pair(G, make_pair(V, undirectedE));
}

// 计算两个有序邻接表的共同邻居
int count_common_neighbors(const vector<int>& adj_u, const vector<int>& adj_v) {
    int i = 0, j = 0, count = 0;
    while (i < adj_u.size() && j < adj_v.size()) {
        if (adj_u[i] == adj_v[j]) {
            count++;
            i++;
            j++;
        } else if (adj_u[i] < adj_v[j]) {
            i++;
        } else {
            j++;
        }
    }
    return count;
}

// 
vector<vector<Edge>> truss_decomposition(const vector<vector<int>>& G, int V, int E) {
    // 准备好：无向图adj， V， E
    // step1 : compute sup(e) for each edge e∈Eg;
    vector<Edge> edges;
    unordered_map<Edge, int, EdgeHash> sup;   // 边 -> 支持度
    int maxSupport = 0;
    // 计算所有边（u < v）初始支持度
    for (int u = 0; u < V; u++) {
        for (int v_idx = 0; v_idx < G[u].size(); v_idx++) {
            int v = G[u][v_idx];
            if (v <= u) continue;
            const vector<int>& adj_u = G[u];
            const vector<int>& adj_v = G[v];
            int s = count_common_neighbors(adj_u, adj_v);
            edges.emplace_back(u, v);
            sup[{u, v}] = s;
            if (s > maxSupport) maxSupport = s;
        }
    }


    // 
    vector<vector<Edge>> classes; // classes[k] 存储k类的边，索引从2开始

    return classes;
}

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     cerr << "Usage: " << argv[0] << " <graph_file>" << endl;
    //     return 1;
    // }

    // string filename = argv[1];
    string filename = "/home/xxx/workspace/data/graph_data/amaze.txt";
    cout << "Processing file: " << filename << endl;
    
    pair<vector<vector<int>>, pair<int, int>> g = readGraph(filename);
    vector<vector<int>> adj = g.first;
    int V = g.second.first;
    int E = g.second.second;


    auto classes = truss_decomposition(adj, V, E);

    cout << " |V|    |E|    kmax" << endl;
    cout << V << "\t" << E << endl;
    return 0;
}

