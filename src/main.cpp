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
        // 使用 std::hash 对两个整数分别哈希，然后组合
        auto hash1 = hash<int>{}(e.first);
        auto hash2 = hash<int>{}(e.second);
        // 组合哈希值的一种方法
        return hash1 ^ (hash2 << 1);
    }
};

vector<int> bucketSort(const vector<int>& neighbors, int maxVertex) {
    vector<bool> bucket(maxVertex + 1, false);
    vector<int> sortedNeighbors;

    // 将邻居节点放入桶中
    for (int neighbor : neighbors) {
        bucket[neighbor] = true;
    }
    
    // 按顺序收集桶中的节点
    for (int i = 0; i <= maxVertex; ++i) {
        if (bucket[i]) {
            sortedNeighbors.push_back(i);
        }
    }
    return sortedNeighbors;
}

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
        
        // 使用桶排序替代快速排序
        G[i] = bucketSort(neighbors, V - 1);
    }

    // 将反向边添加到邻接表中
    for (int u = 0; u < V; u++) {
        // 合并原始邻居和反向邻居
        vector<int> combinedNeighbors = G[u];
        combinedNeighbors.insert(combinedNeighbors.end(), reverseEdges[u].begin(), reverseEdges[u].end());
        
        // 去重并排序
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
    undirectedE /= 2; // 每条无向边被计算了两次

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
    // 对边的支持度捅排序
    vector<vector<Edge>> supportBins(maxSupport + 1);
    for (const auto& e : edges) {
        int s = sup[e];
        supportBins[s].push_back(e);
    }

    // 
    vector<int> binPointers(maxSupport + 1, 0);
    unordered_map<Edge, int, EdgeHash> processed; // 标记边是否已处理
    vector<vector<Edge>> classes; // classes[k] 存储k类的边，索引从2开始

    for (int k = 2; k <= maxSupport + 2; k++) {
        int currentSupport = k - 2;

        // 处理支持度小于等于currentSupport的边
        for(int s = 0; s <= currentSupport; s++) {
            if (s >= supportBins.size()) continue;
            while(binPointers[s] < supportBins[s].size()) {
                Edge e = supportBins[s][binPointers[s]];
                binPointers[s]++; // 移动指针，逻辑上删除当前边

                if (processed.count(e)) continue;
                int actualSupport = sup[e];
                if (actualSupport != s) continue; // 跳过已更新支持度的边

                // 确定该边所属的k-truss
                int edgeK = actualSupport + 2;
                if (edgeK < 2) edgeK = 2;

                // 扩展classes数组到edgeK的大小
                while (classes.size() <= edgeK) {
                    classes.emplace_back();
                }
                classes[edgeK].push_back(e);
                processed[e] = true;

                int u = e.first, v = e.second;
                vector<int> common_neighbors;

                // 查找u和v的共同邻居
                int i = 0, j = 0;
                while (i < G[u].size() && j < G[v].size()) {
                    int a = G[u][i], b = G[v][j];
                    if (a == b && a != u && a != v) { // 排除自身，确保是三角形
                        common_neighbors.push_back(a);
                    }
                    if (a <= b) i++;
                    if (a >= b) j++;
                }

                // 更新相关边的支持度
                for (int w : common_neighbors) {
                    // 处理边(u, w)
                    Edge e1 = (u < w) ? Edge(u, w) : Edge(w, u);
                    if (!processed.count(e1)) {
                        int oldSupport = sup[e1];
                        sup[e1]--;
                        int newSupport = sup[e1];
                        
                        // 如果支持度减少，将边移动到新的桶
                        if (oldSupport != newSupport) {
                            if (newSupport < supportBins.size()) {
                                supportBins[newSupport].push_back(e1);
                            } else {
                                // 扩展桶数组以容纳新的支持度
                                while (supportBins.size() <= newSupport) {
                                    supportBins.emplace_back();
                                }
                                supportBins[newSupport].push_back(e1);
                            }
                        }
                    }
                // 处理边(v, w)
                    Edge e2 = (v < w) ? Edge(v, w) : Edge(w, v);
                    if (!processed.count(e2)) {
                        int oldSupport = sup[e2];
                        sup[e2]--;
                        int newSupport = sup[e2];
                        
                        // 如果支持度减少，将边移动到新的桶
                        if (oldSupport != newSupport) {
                            if (newSupport < supportBins.size()) {
                                supportBins[newSupport].push_back(e2);
                            } else {
                                // 扩展桶数组以容纳新的支持度
                                while (supportBins.size() <= newSupport) {
                                    supportBins.emplace_back();
                                }
                                supportBins[newSupport].push_back(e2);
                            }
                        }
                    }
                }
            }
        }
    }
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


    // auto classes = truss_decomposition(adj, V, E);

    cout << " |V|    |E|    kmax" << endl;
    cout << V << "\t" << E << endl;
    return 0;
}