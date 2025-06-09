#ifndef TRUSSDECOMPOSITION_H
#define TRUSSDECOMPOSITION_H
#include <iostream>
#include <algorithm>
#include <string.h>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>
#include <map>

typedef struct {
    int u, v;
} TEdge;

bool operator<(TEdge e1, TEdge e2);

class TrussDecomposition {
private:
    std::ifstream fin;
    std::string filename;   // 输入的文件

    int V, E;       // 顶点数，边数
    int m;
    int kmax;
    std::vector<int> mapto;     // 胺度数排序后的顶点顺序
    std::vector<int> deg;   // 每个顶点度数
    std::vector<int> bin;   // 桶排序
    std::vector<TEdge> binEdge; // 按支持度排序的边
    std::vector<std::vector<int>> A;    // 邻接表
    std::vector<std::map<int, int>> adj;    // 与顶点邻接的顶点和其支持度
    std::vector<std::map<int, int>> pos;    // 每条边在邻接表中的位置

    inline bool compVertex(int i, int j);   // 比较顶点
    inline void orderPair(int &u, int &v);  // 顶点顺序
    inline void updateSupport(int u, int v, int delta); // 更新支持度
    inline void removeEdge(int u, int v);   // 删除边

    void intersect(const std::vector<int> &a, const std::vector<int> &b, std::vector<int> &c);  // 两个顶点集合的交集

public:
    TrussDecomposition(std::string filename);   // 构造函数
    ~TrussDecomposition();      // 析构函数

    void readGraph();   // 读取文件
    void reorder();     // 重新排序顶点
    void countTriangles();  // 统计三角形个数
    void binSort();  //边支持度 桶排序
    void updateEdge(int u, int v, int minsup);  // 更新边的支持度
    void trussDecomp();     // truss分解
    int getVnum();  
    int getEnum();
    int getKmax();
    void fun();
};

#endif // TRUSSDECOMPOSITION_H