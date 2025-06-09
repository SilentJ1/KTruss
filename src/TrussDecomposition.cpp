#include "TrussDecomposition.h"

using namespace std;

/**
 * 边的比较运算符，用于确定边的顺序
 * 首先比较u顶点，若相同则比较v顶点
 */
bool operator<(TEdge e1, TEdge e2) {
    return e1.u < e2.u || (e1.u == e2.u && e1.v < e2.v);
}

/**
 * 构造函数，初始化图分解器
 * @param filename 输入图数据的文件路径
 */
TrussDecomposition::TrussDecomposition(string filename) 
    : filename(filename) {
    V = E = 0;       // 初始化顶点数和边数
    kmax = 0;        // 初始化最大truss值
}

/**
 * 析构函数，确保文件流关闭
 */
TrussDecomposition::~TrussDecomposition() {
    if (fin.is_open()) fin.close();
}

/**
 * 比较两个顶点的优先级
 * 首先比较度数，度数小的优先；度数相同则比较顶点ID
 */
bool TrussDecomposition::compVertex(int i, int j) {
    return deg[i] < deg[j] || (deg[i] == deg[j] && i < j);
}

/**
 * 确保u和v按compVertex定义的顺序排列
 * 如果u > v，则交换它们的值
 */
void TrussDecomposition::orderPair(int &u, int &v) {
    if (!compVertex(u, v)) swap(u, v);
}

/**
 * 更新边(u,v)的支持度（即包含该边的三角形数量）
 * @param u 边的一个顶点
 * @param v 边的另一个顶点
 * @param delta 支持度的变化量
 */
void TrussDecomposition::updateSupport(int u, int v, int delta) {
    adj[u][v] += delta;
    adj[v][u] += delta;
}

/**
 * 从图中移除边(u,v)
 * @param u 边的一个顶点
 * @param v 边的另一个顶点
 */
void TrussDecomposition::removeEdge(int u, int v) {
    adj[u].erase(v);
    adj[v].erase(u);
}

/**
 * 计算两个有序向量的交集（用于寻找共同邻居）
 * @param a 第一个有序向量
 * @param b 第二个有序向量
 * @param c 存储交集结果的向量
 */
void TrussDecomposition::intersect(const vector<int> &a, const vector<int> &b, vector<int> &c) {
    c.clear();
    unsigned j = 0;
    for (unsigned i = 0; i < a.size(); ++i) {
        // 跳过b中小于a[i]的元素
        // 降序
        while (j < b.size() && b[j] > a[i]) ++j;
        
        if (j >= b.size()) break;
        
        if (b[j] == a[i]) c.push_back(a[i]);
    }
}

/**
 * 从文件读取图数据
 */
void TrussDecomposition::readGraph() {
    fin.open(filename.c_str());
    if (!fin.is_open()) {
        cerr << "Error opening input file!" << endl;
        exit(1);
    }

    // 读取顶点数和边数
    fin >> V >> E;
    deg.resize(V, 0);     // 初始化度数数组
    adj.resize(V);        // 初始化邻接表
    
    // 读取每个顶点的邻接表
    for (int u = 0; u < V; ++u) {
        int out_degree;
        fin >> out_degree >> out_degree;  // 读取两次
        for (int j = 0; j < out_degree; ++j) {
            int v;
            fin >> v;
            // 避免重复添加边
            if (adj[u].find(v) == adj[u].end()) {
                adj[u][v] = 0;  // 初始化支持度为0
                adj[v][u] = 0;
                ++deg[u];       // 更新顶点度数
                ++deg[v];
            }
        }
    }
    m = E;  // m表示实际处理的边数
    fin.close();
}

/**
 * 对顶点进行重排序
 * 按照顶点度数从小到大排序，度数相同则按ID排序
 */
void TrussDecomposition::reorder() {
    mapto.resize(V);
    for (int i = 0; i < V; ++i) mapto[i] = i;
    // 使用lambda表达式进行排序
    sort(mapto.begin(), mapto.end(), [this](int i, int j) {
        return compVertex(i, j);
    });
}

/**
 * 计算图中每个边的三角形数量（支持度）
 * 使用重排序后的顶点顺序，从后向前处理每个顶点
 */
void TrussDecomposition::countTriangles() {
    // A[u]存储的是顶点u的邻居顶点在mapto数组中的索引
    A.resize(V);  // 初始化辅助邻接表
    
    // 按重排序后的顺序处理顶点
    for (int vi = V - 1; vi >= 0; --vi) {
        int v = mapto[vi];
        for (auto it = adj[v].begin(); it != adj[v].end(); ++it) {
            int u = it->first;
            if (!compVertex(u, v)) continue;  // 确保u < v
            
            // 计算u和v的共同邻居
            vector<int> common;
            intersect(A[u], A[v], common);
            
            // 每找到一个共同邻居，就形成一个三角形
            for (unsigned i = 0; i < common.size(); ++i) {
                int w = mapto[common[i]];
                updateSupport(u, v, 1);  // 更新边的支持度
                updateSupport(v, w, 1);
                updateSupport(w, u, 1);
            }
            
            // 将当前顶点索引添加到u的辅助邻接表中
            A[u].push_back(vi);
        }
    }
}

/**
 * 对边进行桶排序，按支持度从小到大排列
 */
void TrussDecomposition::binSort() {
    // bin数组用于记录每个支持度对应的边的数量
    bin.clear();
    bin.resize(V, 0);
    // nBins记录最大支持度值
    // mp记录实际处理的边数
    int nBins = 0, mp = 0;

    // 统计每个支持度出现的次数
    for (int u = 0; u < V; ++u) {
        // tadj : map<int, int>
        auto tadj = adj[u];
        for (auto it = tadj.begin(); it != tadj.end(); ++it) {
            int v = it->first;
            if (!compVertex(u, v)) continue;
            int sup = it->second;
            if (sup == 0) {
                removeEdge(u, v);  // 移除支持度为0的边
                continue;
            }
            ++mp;
            ++bin[sup];
            nBins = max(sup, nBins);
        }
    }
    m = mp;  // 更新实际处理的边数
    ++nBins;

    // 计算每个桶的起始位置
    int count = 0;
    for (int i = 0; i < nBins; ++i) {
        int binsize = bin[i];
        bin[i] = count;
        count += binsize;
    }

    // 将边放入对应的桶中
    // pos数组记录每条边在binEdge中的位置
    pos.clear();
    pos.resize(V);
    binEdge.resize(m);

    for (int u = 0; u < V; ++u)
        for (auto it = adj[u].begin(); it != adj[u].end(); ++it) {
            int v = it->first;
            if (!compVertex(u, v)) continue;
            int sup = it->second;
            TEdge e = {u, v};
            int &b = bin[sup];
            binEdge[b] = e;
            pos[u][v] = b++;
        }

    // 调整桶的起始位置
    for (int i = nBins; i > 0; --i) {
        bin[i] = bin[i - 1];
    }
    bin[0] = 0;
}

/**
 * 更新边的支持度，并在需要时调整边在桶中的位置
 * @param u 边的一个顶点
 * @param v 边的另一个顶点
 * @param minsup 当前处理的最小支持度阈值
 */
void TrussDecomposition::updateEdge(int u, int v, int minsup) {
    orderPair(u, v);
    int sup = adj[u][v];
    if (sup <= minsup) return;  // 如果支持度已经小于等于阈值，则不处理

    int p = pos[u][v];          // 当前边在数组中的位置
    int posbin = bin[sup];      // 当前支持度桶的起始位置
    TEdge se = binEdge[posbin]; // 桶起始位置的边
    TEdge e = {u, v};

    // 交换当前边和桶起始位置的边
    if (p != posbin) {
        pos[u][v] = posbin;
        pos[se.u][se.v] = p;
        binEdge[p] = se;
        binEdge[posbin] = e;
    }
    
    // 调整桶的起始位置
    ++bin[sup];
    // 减少边的支持度
    updateSupport(u, v, -1);
}

/**
 * 执行k-truss分解的核心算法
 * 逐步移除支持度最低的边，并更新相关边的支持度
 */
void TrussDecomposition::trussDecomp() {
    kmax = 0; // 初始化最大truss值
    
    // 处理每条边
    for (int s = 0; s < m; ++s) {
        int u = binEdge[s].u;
        int v = binEdge[s].v;
        orderPair(u, v);
        int supuv = adj[u][v];
        
        // 计算当前边的truss值（k = 支持度 + 2）
        int currentTruss = supuv + 2;
        if (currentTruss > kmax) {
            kmax = currentTruss;  // 更新最大truss值
        }

        // 查找所有与u和v都相邻的顶点w，并更新它们的支持度
        int nfound = 0;
        for (auto it = adj[u].begin(); it != adj[u].end(); ++it) {
            if (nfound == supuv) break;  // 已经找到足够的邻居
            int w = it->first;
            if (w == v) continue;  // 跳过v自身
            if (adj[v].find(w) != adj[v].end()) {
                ++nfound;
                updateEdge(u, w, supuv);  // 更新边(u,w)的支持度
                updateEdge(v, w, supuv);  // 更新边(v,w)的支持度
            }
        }
        
        // 处理完当前边后，将其从图中移除
        removeEdge(u, v);
    }
}

// 获取顶点数量
int TrussDecomposition::getVnum()
{
    return V;
}

// 获取边的数量
int TrussDecomposition::getEnum()
{
    return E;
}

// 获取最大truss值
int TrussDecomposition::getKmax()
{
    return kmax;
}

/**
 * 执行完整的truss分解流程
 * 依次调用读取图、重排序、计算三角形、桶排序和truss分解
 */
void TrussDecomposition::fun()
{
    readGraph();
    reorder();
    countTriangles();
    binSort();
    trussDecomp();
}