// Truss.h
#ifndef TRUSSDECOMPOSITION_H
#define TRUSSDECOMPOSITION_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

typedef std::pair<int, int> Edge;
const int maxClass = 1000;  // 最大 K-class 类别


class TrussDecomposition {
public:
    void readGraph(std::string filename);
    int getVnum();
    int getEnum();


private:
    int V;                      // 顶点数量
    int E;                      // 边数量
    std::vector<std::vector<int>> A;  // 邻接顶点
    std::vector<std::map<int, int>> adj;    //存储图的邻接矩阵信息，包括边的支持度
    std::vector<int> deg;       // 每个顶点的度数

    std::vector<int> mapto;     // 存储顶点的重新排序映射
    std::vector<int> bin;       // 存储每个桶的起始位置
    std::vector<Edge> binEdge;  // 经过桶排序后的边
    std::vector<std::map<int, int>> pos;    //存储每条边在 binEdge 中的位置
    int cntClass[maxClass];
};

#endif // TRUSS_DECOMPOSITION_H