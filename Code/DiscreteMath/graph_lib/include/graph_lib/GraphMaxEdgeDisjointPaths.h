#pragma once

#include "GraphMaxFlow.h"

template<typename T, bool allowLoops>
T GraphMaxEdgeDisjointPaths(const GraphAdjacencyMatrix<T, true, allowLoops>& weightedGraph, size_t from, size_t to) {
    /*
        This problem can be transformed to a maximum flow problem by constructing a network N = (V, E) from G,
        with s and t being the source and the sink of N respectively, and assigning each edge a capacity of 1.
        In this network, the maximum flow is k if there are k edge - disjoint paths.
     */

    auto graph = weightedGraph;
    const size_t verticesCount = weightedGraph.GetVerticesCount();
    for (size_t i = 0; i < verticesCount; ++i) {
        for (size_t j = 0; j < verticesCount; ++j) {
            if (i != j) {
                if (auto& edge = graph.GetEdge(i, j); edge != 0) {
                    edge = T{ 1 };
                }
            }
        }
    }

    return GraphMaxFlow(graph, from, to);
}
