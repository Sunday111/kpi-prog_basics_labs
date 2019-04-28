#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>

#include "graph_lib/AdjacencyMatrix.h"
#include "graph_lib/ReadGraph.h"

struct VisitedVertex
{
    size_t index;
    size_t weight;
};

int main() {
    //std::istream& input = std::cin;
    std::ostream* output = &std::cout;

    std::string sample_input =
    /* vertices count */"8 \n"
    /*     0 1 2 3 4 5 6 7 */
    /*0*/ "1 1 1 0 1 0 0 0 \n"
    /*1*/ "0 1 0 0 0 0 0 1 \n"
    /*2*/ "0 1 1 1 0 0 0 0 \n"
    /*3*/ "0 0 0 1 0 0 0 0 \n"
    /*4*/ "0 0 0 0 1 1 0 0 \n"
    /*5*/ "0 0 0 0 0 1 1 0 \n"
    /*6*/ "1 0 0 0 0 0 1 0 \n"
    /*7*/ "0 0 0 0 0 0 1 1 \n"
    /* start vertex */"0 \n"
    /* end vertex */"6 \n";

    std::stringstream input(sample_input);

    try
    {
        auto graph = ReadGraph<bool, true, true>(input, output);
        const size_t startIndex = MustReadValue<size_t>(input, "Enter start vertex index: ");
        const size_t endIndex = MustReadValue<size_t>(input, "Enter end vertex index: ");

        std::vector<VisitedVertex> visitedVertices;
        auto findVisited = [&visitedVertices](size_t index) {
            return std::lower_bound(visitedVertices.begin(), visitedVertices.end(), index, [](const VisitedVertex& value, size_t index) {
                return value.index < index;
            });
        };
        
        // Assign vertices weights
        size_t currentWeight = 0;
        {
            // queue of vertex indices
            std::queue<size_t> walkQueue;
            walkQueue.push(startIndex);
            bool pathFound = false;
            while (!walkQueue.empty() && !pathFound) {
                // indices with same weight
                const size_t n = walkQueue.size();
                for (size_t ni = 0; ni < n && !pathFound; ++ni) {
                    const size_t from = walkQueue.front();
                    walkQueue.pop();
                    auto fromIt = findVisited(from);
                    if (fromIt == visitedVertices.end() || fromIt->index != from) {
                        fromIt = visitedVertices.insert(fromIt, VisitedVertex{ from, currentWeight });
                        for (size_t to = 0; to < graph.GetVerticesCount(); ++to) {
                            if (from != to && graph.GetEdge(from, to)) {
                                if (to == endIndex) {
                                    pathFound = true;
                                    break;
                                }

                                auto toIt = findVisited(to);
                                if (toIt == visitedVertices.end() || toIt->index != to) {
                                    walkQueue.push(to);
                                }
                            }
                        }
                    }
                }

                ++currentWeight;
            }
        }

        // Build path from endIndex to startIndex by computed weights
        std::vector<size_t> path;
        path.push_back(endIndex);
        while (path.back() != startIndex) {
            --currentWeight;
            const size_t last = path.back();
            for (size_t i = 0; i < graph.GetVerticesCount(); ++i) {
                if (i != last && graph.GetEdge(i, last)) {
                    auto it = findVisited(i);
                    if (it != visitedVertices.end() && it->index == i) {
                        if (it->weight == currentWeight) {
                            path.push_back(i);
                            break;
                        }
                    }
                }
            }
        }

        // Print path
        *output << "Path: ";
        for (size_t i = 0; i < path.size(); ++i) {
            const size_t vertex = path[i];
            *output << vertex;
            if (i < path.size() - 1) {
                *output << " -> ";
            }
        }
        *output << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
