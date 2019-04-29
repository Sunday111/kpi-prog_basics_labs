#pragma once

#include <algorithm>

#include "graph_lib/AdjacencyMatrix.h"

template<typename T, bool allowLoops>
T GraphMaxFlow(const GraphAdjacencyMatrix<T, true, allowLoops>& graph, size_t startIndex, size_t endIndex) {
    const size_t verticesCount = graph.GetVerticesCount();
    auto edgesFlow = [&, values = std::vector<T>(verticesCount * verticesCount, T{})]
    (size_t i, size_t j) mutable->T& {
        Assert(i < verticesCount && j < verticesCount, "Index out of range");
        const size_t offset = i * verticesCount;
        const size_t index = offset + j;
        return values[index];
    };

    struct VisitedVertex
    {
        bool forward;
        size_t index;
        size_t prev;
        T reserve;
    };

    std::vector<VisitedVertex> visitedVertices;

    auto visitedLowerBound = [&](size_t v) {
        return std::lower_bound(visitedVertices.begin(), visitedVertices.end(), v, [](const VisitedVertex& vv, size_t v) {
            return vv.index < v;
        });
    };

    auto visitedFound = [&](auto iterator, size_t v) {
        return iterator != visitedVertices.end() && iterator->index == v;
    };

    size_t lastVisited;
    do
    {
        visitedVertices.clear();

        {
            VisitedVertex initial;
            initial.index = startIndex;
            initial.reserve = std::numeric_limits<T>::max();
            visitedVertices.push_back(initial);
        }

        // Build path from startIndex to endIndex
        lastVisited = startIndex;
        bool nextVertexFound;
        do
        {
            const size_t v = lastVisited;
            auto vit = visitedLowerBound(v);
            Assert(visitedFound(vit, v), "Internal error: visited vertex isn't added to visitedVertices");
            nextVertexFound = false;
            // iterate through edges from lastVisited vertex and take any valid
            for (size_t w = 0; w < verticesCount; ++w) {
                if (auto wit = visitedLowerBound(w); !visitedFound(wit, w)) {
                    VisitedVertex visitedVertex;
                    // try take 'v' -> 'w' edge
                    if (const T& vwCapacity = graph.GetEdgeConst(v, w); vwCapacity > 0) {
                        const T& vwFlow = edgesFlow(v, w);
                        if (vwFlow < vwCapacity) {
                            visitedVertex.reserve = std::min(vwCapacity - vwFlow, vit->reserve);
                            visitedVertex.forward = true;
                            lastVisited = w;
                        }
                    }
                    // try take 'w' -> 'v' edge
                    else if (const T& wvCapacity = graph.GetEdgeConst(w, v); wvCapacity > 0) {
                        const T& wvFlow = edgesFlow(w, v);
                        if (wvFlow > 0) {
                            visitedVertex.reserve = std::min(wvFlow, vit->reserve);
                            visitedVertex.forward = false;
                            lastVisited = w;
                        }
                    }

                    if (lastVisited == w) {
                        visitedVertex.index = w;
                        visitedVertex.prev = v;
                        wit = visitedVertices.insert(wit, visitedVertex);
                        nextVertexFound = true;
                        break;
                    }
                }
            }
        } while (lastVisited != endIndex && nextVertexFound);

        // Update flow on edges
        if (lastVisited == endIndex) {
            auto it = visitedLowerBound(endIndex);
            Assert(visitedFound(it, endIndex), "Internal error: visited vertex isn't added to visitedVertices");
            const auto& reserve = it->reserve;

            do
            {
                if (it->forward) {
                    T& edgeFlow = edgesFlow(it->prev, it->index);
                    edgeFlow += reserve;
                }
                else {
                    T& edgeFlow = edgesFlow(it->index, it->prev);
                    Assert(reserve <= edgeFlow, "Internal logic error");
                    edgeFlow -= reserve;
                }

                auto nextIt = visitedLowerBound(it->prev);
                Assert(visitedFound(nextIt, it->prev), "Internal logic error");
                it = nextIt;
            } while (it->index != startIndex);
        }
    } while (lastVisited == endIndex);

    size_t maxFlow = 0;
    for (size_t i = 0; i < verticesCount; ++i) {
        auto& edge = graph.GetEdgeConst(startIndex, i);
        maxFlow += edgesFlow(startIndex, i);
    }

    return maxFlow;
}
