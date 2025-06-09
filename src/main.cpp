#include "TrussDecomposition.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <graph_file>" << endl;
        return 1;
    }

    string filename = argv[1];
    // string filename = "/home/xxx/workspace/data/graph_data/amaze.txt";
    cout << "Processing file: " << filename << endl;
    
    // 计时开始
    auto start = high_resolution_clock::now();
    
    TrussDecomposition td(filename);
    td.fun();
    
    // 计时结束
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    cout << " |V|\t|E|\ttime(ms)\tkmax" << endl;
    cout << td.getVnum() << "\t" << td.getEnum() << "\t" << duration << "\t\t" << td.getKmax() << endl;
    return 0;
}