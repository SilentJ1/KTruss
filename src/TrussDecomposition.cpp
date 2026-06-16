#include "TrussDecomposition.h"

#include <algorithm>
#include <fstream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ktruss {
namespace {

struct PairHash {
    std::size_t operator()(const Edge& edge) const {
        const auto first = static_cast<std::size_t>(edge.first);
        const auto second = static_cast<std::size_t>(edge.second);
        return (first << 32U) ^ second;
    }
};

struct QueueItem {
    int support;
    int edge_id;

    bool operator>(const QueueItem& other) const {
        if (support != other.support) {
            return support > other.support;
        }
        return edge_id > other.edge_id;
    }
};

std::vector<long long> parseIntegerLine(const std::string& line) {
    std::istringstream stream(line);
    std::vector<long long> values;
    long long value = 0;
    while (stream >> value) {
        values.push_back(value);
    }
    return values;
}

Edge canonicalEdge(long long raw_u, long long raw_v, bool one_based) {
    raw_u -= one_based ? 1 : 0;
    raw_v -= one_based ? 1 : 0;
    if (raw_u < 0 || raw_v < 0) {
        throw std::out_of_range("vertex id is negative after base conversion");
    }
    if (raw_u == raw_v) {
        return {-1, -1};
    }

    Vertex u = static_cast<Vertex>(raw_u);
    Vertex v = static_cast<Vertex>(raw_v);
    if (u > v) {
        std::swap(u, v);
    }
    return {u, v};
}

bool looksLikeAdjacencyRows(const std::vector<std::vector<long long>>& rows) {
    if (rows.empty()) {
        return false;
    }

    bool id_rows = true;
    bool degree_rows = true;
    for (const auto& row : rows) {
        if (row.empty()) {
            return false;
        }
        if (row.size() < 2 || row[1] != static_cast<long long>(row.size() - 2)) {
            id_rows = false;
        }
        if (row[0] != static_cast<long long>(row.size() - 1)) {
            degree_rows = false;
        }
    }
    return id_rows || degree_rows;
}

GraphData graphFromEdges(std::vector<Edge> edges, std::size_t declared_vertices = 0) {
    std::sort(edges.begin(), edges.end());
    edges.erase(std::remove(edges.begin(), edges.end(), Edge{-1, -1}), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

    std::size_t vertex_count = declared_vertices;
    for (const auto& [u, v] : edges) {
        vertex_count = std::max(vertex_count, static_cast<std::size_t>(std::max(u, v) + 1));
    }
    return {vertex_count, std::move(edges)};
}

GraphData parseAdjacency(const std::vector<std::vector<long long>>& rows, std::size_t vertex_count, bool one_based) {
    const bool use_id_rows = std::all_of(rows.begin(), rows.end(), [](const auto& row) {
        return row.size() >= 2 && row[1] == static_cast<long long>(row.size() - 2);
    });

    std::vector<Edge> edges;
    for (std::size_t row_index = 0; row_index < rows.size(); ++row_index) {
        const auto& row = rows[row_index];
        std::size_t source_index = row_index;
        std::size_t degree_index = 0;
        std::size_t first_neighbor = 1;

        if (use_id_rows) {
            long long raw_source = row[0] - (one_based ? 1 : 0);
            if (raw_source < 0) {
                throw std::out_of_range("adjacency source id is negative after base conversion");
            }
            source_index = static_cast<std::size_t>(raw_source);
            degree_index = 1;
            first_neighbor = 2;
        }

        if (source_index >= vertex_count) {
            throw std::out_of_range("adjacency source id is outside [0, |V|)");
        }

        const auto degree = static_cast<std::size_t>(row[degree_index]);
        if (first_neighbor + degree != row.size()) {
            throw std::runtime_error("adjacency row degree does not match neighbor count");
        }

        for (std::size_t i = first_neighbor; i < row.size(); ++i) {
            long long target = row[i] - (one_based ? 1 : 0);
            Edge edge = canonicalEdge(static_cast<long long>(source_index), target, false);
            if (edge.first != -1 && static_cast<std::size_t>(edge.second) >= vertex_count) {
                throw std::out_of_range("adjacency neighbor id is outside [0, |V|)");
            }
            edges.push_back(edge);
        }
    }

    return graphFromEdges(std::move(edges), vertex_count);
}

std::vector<Vertex> commonNeighbors(Vertex u, Vertex v, const std::vector<std::unordered_set<Vertex>>& adjacency) {
    const auto& a = adjacency[u];
    const auto& b = adjacency[v];
    const auto& smaller = a.size() <= b.size() ? a : b;
    const auto& larger = a.size() <= b.size() ? b : a;

    std::vector<Vertex> common;
    common.reserve(std::min(a.size(), b.size()));
    for (Vertex w : smaller) {
        if (larger.find(w) != larger.end()) {
            common.push_back(w);
        }
    }
    return common;
}

}  // namespace

GraphData readGraphFile(const std::string& filename, bool one_based) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("failed to open graph file: " + filename);
    }

    std::vector<std::vector<long long>> rows;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::vector<long long> values = parseIntegerLine(line);
        if (!values.empty()) {
            rows.push_back(std::move(values));
        }
    }
    if (rows.empty()) {
        throw std::runtime_error("empty graph file: " + filename);
    }

    if (rows.front().size() == 2 && rows.size() > 1) {
        const long long declared_vertices = rows.front()[0];
        const long long declared_edges = rows.front()[1];
        if (declared_vertices > 0 && declared_edges >= 0) {
            std::vector<std::vector<long long>> rest(rows.begin() + 1, rows.end());
            if (looksLikeAdjacencyRows(rest)) {
                return parseAdjacency(rest, static_cast<std::size_t>(declared_vertices), one_based);
            }
            if (static_cast<std::size_t>(declared_edges) == rest.size()) {
                std::vector<Edge> edges;
                edges.reserve(rest.size());
                for (const auto& edge_row : rest) {
                    if (edge_row.size() < 2) {
                        throw std::runtime_error("edge-list row must contain '<source> <target>'");
                    }
                    edges.push_back(canonicalEdge(edge_row[0], edge_row[1], one_based));
                }
                return graphFromEdges(std::move(edges), static_cast<std::size_t>(declared_vertices));
            }
        }
    }

    std::vector<Edge> edges;
    edges.reserve(rows.size());
    for (const auto& edge_row : rows) {
        if (edge_row.size() < 2) {
            throw std::runtime_error("edge-list row must contain '<source> <target>'");
        }
        edges.push_back(canonicalEdge(edge_row[0], edge_row[1], one_based));
    }
    return graphFromEdges(std::move(edges));
}

DecompositionResult decompose(const GraphData& graph) {
    std::vector<std::unordered_set<Vertex>> adjacency(graph.vertex_count);
    std::unordered_map<Edge, int, PairHash> edge_id;
    edge_id.reserve(graph.edges.size() * 2 + 1);

    for (std::size_t i = 0; i < graph.edges.size(); ++i) {
        const auto [u, v] = graph.edges[i];
        if (u < 0 || v < 0 || static_cast<std::size_t>(v) >= graph.vertex_count) {
            throw std::out_of_range("edge endpoint is outside [0, |V|)");
        }
        adjacency[u].insert(v);
        adjacency[v].insert(u);
        edge_id[graph.edges[i]] = static_cast<int>(i);
    }

    std::vector<int> support(graph.edges.size(), 0);
    for (std::size_t i = 0; i < graph.edges.size(); ++i) {
        const auto [u, v] = graph.edges[i];
        support[i] = static_cast<int>(commonNeighbors(u, v, adjacency).size());
    }

    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> queue;
    for (std::size_t i = 0; i < support.size(); ++i) {
        queue.push({support[i], static_cast<int>(i)});
    }

    std::vector<bool> active(graph.edges.size(), true);
    DecompositionResult result;
    result.vertex_count = graph.vertex_count;
    result.edge_count = graph.edges.size();
    result.edge_truss.resize(graph.edges.size());

    while (!queue.empty()) {
        const QueueItem item = queue.top();
        queue.pop();

        const int id = item.edge_id;
        if (!active[id] || item.support != support[id]) {
            continue;
        }

        const auto [u, v] = graph.edges[id];
        const int removed_support = support[id];
        const int truss = removed_support + 2;
        result.kmax = std::max(result.kmax, truss);
        result.edge_truss[id] = {graph.edges[id], truss};

        const std::vector<Vertex> common = commonNeighbors(u, v, adjacency);
        for (Vertex w : common) {
            const Edge uw = u < w ? Edge{u, w} : Edge{w, u};
            const Edge vw = v < w ? Edge{v, w} : Edge{w, v};
            const auto uw_it = edge_id.find(uw);
            const auto vw_it = edge_id.find(vw);
            if (uw_it == edge_id.end() || vw_it == edge_id.end()) {
                continue;
            }

            const int uw_id = uw_it->second;
            const int vw_id = vw_it->second;
            if (active[uw_id] && support[uw_id] > removed_support) {
                --support[uw_id];
                queue.push({support[uw_id], uw_id});
            }
            if (active[vw_id] && support[vw_id] > removed_support) {
                --support[vw_id];
                queue.push({support[vw_id], vw_id});
            }
        }

        active[id] = false;
        adjacency[u].erase(v);
        adjacency[v].erase(u);
    }

    return result;
}

void writeEdgeTrussFile(const std::string& filename, const DecompositionResult& result) {
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + filename);
    }

    for (const EdgeTruss& item : result.edge_truss) {
        output << item.edge.first << ' ' << item.edge.second << ' ' << item.truss << '\n';
    }
}

}  // namespace ktruss
