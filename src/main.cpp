// main.cpp
#include <iostream>
#include <fstream>
#include "TrussDecomposition.h"

using namespace std;

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     cerr << "Usage: " << argv[0] << " <graph_file>" << endl;
    //     return 1;
    // }
    // string filename = argv[1];
    string filename = "/home/xxx/workspace/data/graph_data/amaze.txt";
    cout << "Processing file: " << filename << endl;
    
    TrussDecomposition t;
    t.readGraph(filename);

    cout << " |V|    |E|    kmax" << endl;
    cout << t.getVnum() << "\t" << t.getEnum() << endl;
    return 0;
}