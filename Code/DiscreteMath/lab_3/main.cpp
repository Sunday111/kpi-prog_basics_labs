#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>

#include "graph_lib/AdjacencyMatrix.h"
#include "graph_lib/ReadGraph.h"

namespace max_graph_flow_impl
{
    template<typename T>
    struct VisitedVertex
    {
        bool forward;
        size_t index;
        size_t prev;
        T reserve;
    };

    template<typename T>
    struct Edge
    {
        T flow = 0;
        T capacity = 0;
    };
}

template<typename T, bool allowLoops>
T MaxGraphFlow(const GraphAdjacencyMatrix<T, true, allowLoops>& weightedGraph, size_t startIndex, size_t endIndex) {
    using namespace max_graph_flow_impl;
    const size_t verticesCount = weightedGraph.GetVerticesCount();
    // Make adjacency matrix with additional info for each edge
    GraphAdjacencyMatrix<Edge<T>, true, allowLoops> graph(verticesCount);

    //auto edgeFlow = [&, values = std::vector<T>(verticesCount * verticesCount)]
    //(size_t i, size_t j) mutable -> T& {
    //    Assert(i < verticesCount && j < verticesCount, "Index out of range");
    //    const size_t offset = i * verticesCount;
    //    const size_t index = offset + j;
    //    return values[index];
    //};

    for (size_t i = 0; i < verticesCount; ++i) {
        for (size_t j = 0; j < verticesCount; ++j) {
            if (i != j) {
                auto& edge = graph.GetEdge(i, j);
                edge.capacity = weightedGraph.GetEdgeConst(i, j);
            }
        }
    }

    std::vector<VisitedVertex<T>> visitedVertices;

    auto visitedLowerBound = [&](size_t v) {
        return std::lower_bound(visitedVertices.begin(), visitedVertices.end(), v, [](const VisitedVertex<T>& vv, size_t v) {
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
            VisitedVertex<T> initial;
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
                    VisitedVertex<T> visitedVertex;
                    // try take 'v' -> 'w' edge
                    if (const Edge<T>& vw = graph.GetEdgeConst(v, w); vw.capacity > 0) {
                        if (vw.flow < vw.capacity) {
                            visitedVertex.reserve = std::min(vw.capacity - vw.flow, vit->reserve);
                            visitedVertex.forward = true;
                            lastVisited = w;
                        }
                    }
                    // try take 'w' -> 'v' edge
                    else if (const Edge<T>& wv = graph.GetEdgeConst(w, v); wv.capacity > 0) {
                        if (vw.flow > 0) {
                            visitedVertex.reserve = std::min(vw.flow, vit->reserve);
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
            assert(visitedFound(it, endIndex));
            const auto& reserve = it->reserve;

            do
            {
                if (it->forward) {
                    Edge<T>& edge = graph.GetEdge(it->prev, it->index);
                    edge.flow += reserve;
                }
                else {
                    Edge<T>& edge = graph.GetEdge(it->index, it->prev);
                    assert(reserve <= edge.flow);
                    edge.flow -= reserve;
                }

                auto nextIt = visitedLowerBound(it->prev);
                assert(visitedFound(nextIt, it->prev));
                it = nextIt;
            } while (it->index != startIndex);
        }
    } while (lastVisited == endIndex);

    size_t maxFlow = 0;
    for (size_t i = 0; i < verticesCount; ++i) {
        auto& edge = graph.GetEdgeConst(startIndex, i);
        maxFlow += edge.flow;
    }

    return maxFlow;
}

void MaxGraphFlowApp(std::istream& input, std::ostream* output) {
    auto graph = ReadGraph<size_t, true, true>(input, output);
    const size_t startIndex = MustReadValue<size_t>(input, "Enter source vertex index: ");
    const size_t endIndex = MustReadValue<size_t>(input, "Enter sink vertex index: ");
    const auto maxFlow = MaxGraphFlow(graph, startIndex, endIndex);
    *output << "Max flow: " << maxFlow << '\n';
}

void MaxGraphFlowAppTest() {
    //std::istream& input = std::cin;
    std::ostream* output = &std::cout;

    std::string sample_input =
    /* vertices count */"5 \n"
    /*     0 1 2 3 4 */
    /*0*/ "0 2 0 0 5 \n"
    /*1*/ "0 0 3 0 0 \n"
    /*2*/ "0 0 0 3 2 \n"
    /*3*/ "3 3 0 0 3 \n"
    /*4*/ "0 0 0 0 0 \n"
    /* start vertex */"0 \n"
    /* end vertex */"4 \n";

    std::stringstream input(sample_input);

    try
    {
        MaxGraphFlowApp(input, output);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

template<typename T, bool allowLoops>
T MaxEdgeDisjointPaths(const GraphAdjacencyMatrix<T, true, allowLoops>& weightedGraph, size_t startIndex, size_t endIndex) {
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

    return MaxGraphFlow(graph, startIndex, endIndex);
}

void MaxEdgeDisjointPathsApp(std::istream& input, std::ostream* output) {
    auto graph = ReadGraph<size_t, true, true>(input, output);
    const size_t startIndex = MustReadValue<size_t>(input, "Enter source vertex index: ");
    const size_t endIndex = MustReadValue<size_t>(input, "Enter sink vertex index: ");
    const auto maxPaths = MaxEdgeDisjointPaths(graph, startIndex, endIndex);
    *output << "Max edge disjoint paths: " << maxPaths << '\n';
}

void MaxEdgeDisjointPathsTest() {
    //std::istream& input = std::cin;
    std::ostream* output = &std::cout;

    std::string sample_input =
        /* vertices count */"5 \n"
        /*     0 1 2 3 4 */
        /*0*/ "0 2 0 0 5 \n"
        /*1*/ "0 0 3 0 0 \n"
        /*2*/ "0 0 0 3 2 \n"
        /*3*/ "3 3 0 0 3 \n"
        /*4*/ "0 0 0 0 0 \n"
        /* start vertex */"0 \n"
        /* end vertex */"4 \n";

    std::stringstream input(sample_input);

    try
    {
        MaxEdgeDisjointPathsApp(input, output);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

int main() {
    //MaxGraphFlowAppTest();
    MaxEdgeDisjointPathsTest();
    return 0;
}
