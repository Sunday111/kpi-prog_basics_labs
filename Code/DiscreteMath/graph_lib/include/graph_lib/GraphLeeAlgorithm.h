#pragma once

#include "AdjacencyMatrix.h"

template<bool oriented, bool allowLoops>
void GraphBuilVertexWaveNumbers(const GraphAdjacencyMatrix<bool, oriented, allowLoops>& graph, std::vector<std::optional<size_t>>& visited, size_t startVertex, std::optional<size_t> endVertex = std::nullopt) {
    visited.resize(graph.GetVerticesCount());
    // Assign vertices weights
    size_t currentWeight = 0;
    // queue of vertex indices
    std::queue<size_t> walkQueue;
    walkQueue.push(startVertex);
    bool endFound = false;
    while (!walkQueue.empty() && !endFound) {
        // indices with same weight
        const size_t n = walkQueue.size();
        for (size_t ni = 0; ni < n && !endFound; ++ni) {
            const size_t from = walkQueue.front();
            walkQueue.pop();
            if (!visited[from].has_value()) {
                visited[from] = currentWeight;
                for (size_t to = 0; to < graph.GetVerticesCount(); ++to) {
                    if (from != to && graph.GetEdgeConst(from, to)) {
                        if (!visited[to].has_value()) {
                            walkQueue.push(to);
                            if (endVertex.has_value() && to == *endVertex) {
                                endFound = true;
                                break;
                            }
                        }

                    }
                }
            }
        }

        ++currentWeight;
    }
}

template<bool oriented, bool allowLoops>
bool FindPathByWaveNumbers(const GraphAdjacencyMatrix<bool, oriented, allowLoops>& graph, std::vector<std::optional<size_t>>& visited, std::vector<size_t>& path, size_t from, size_t to) {
    Assert(visited[from].has_value() && visited[from] == 0, "Start vertex must have wave number 0");
    if (!visited[to].has_value()) {
        return false;
    }

    size_t currentWeight = *visited[to];
    path.push_back(to);
    while (path.back() != from) {
        const size_t last = path.back();
        for (size_t i = 0; i < graph.GetVerticesCount(); ++i) {
            if (i != last && graph.GetEdgeConst(i, last)) {
                if (visited[i].has_value()) {
                    if (*visited[i] == currentWeight) {
                        path.push_back(i);
                        break;
                    }
                }
            }
        }
        --currentWeight;
    }

    std::reverse(path.begin(), path.end());
    return true;
}