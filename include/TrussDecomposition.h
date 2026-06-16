#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace ktruss {

using Vertex = int;
using Edge = std::pair<Vertex, Vertex>;

struct GraphData {
    std::size_t vertex_count = 0;
    std::vector<Edge> edges;
};

struct EdgeTruss {
    Edge edge;
    int truss = 0;
};

struct DecompositionResult {
    std::size_t vertex_count = 0;
    std::size_t edge_count = 0;
    int kmax = 0;
    std::vector<EdgeTruss> edge_truss;
};

GraphData readGraphFile(const std::string& filename, bool one_based = false);
DecompositionResult decompose(const GraphData& graph);
void writeEdgeTrussFile(const std::string& filename, const DecompositionResult& result);

}  // namespace ktruss
