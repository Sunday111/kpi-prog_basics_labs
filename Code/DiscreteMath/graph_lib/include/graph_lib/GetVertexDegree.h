#pragma once

#include "AdjacencyMatrix.h"

template<bool oriented = false, bool allowLoops = false>
[[nodiscard]] size_t GetVertexDegree(const GraphAdjacencyMatrix<bool, oriented, allowLoops>& g, size_t v) {
    size_t degree = 0;
    for (size_t i = 0; i < g.GetVerticesCount(); ++i) {
        if constexpr (!oriented && !allowLoops) {
            if (i == v) {
                continue;
            }
        }

        if (g.GetEdge(v, i)) {
            ++degree;
        }
    }

    return degree;
}