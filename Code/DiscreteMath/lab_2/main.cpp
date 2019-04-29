#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <optional>

#include "graph_lib/ReadGraph.h"
#include "graph_lib/GraphLeeAlgorithm.h"

void LeeAlgorithmApp(std::istream& input, std::ostream* output) {
    auto graph = ReadGraph<bool, true, true>(input, output);
    const size_t from = MustReadValue<size_t>(input, "Enter start vertex index: ");
    const size_t to = MustReadValue<size_t>(input, "Enter end vertex index: ");

    std::vector<std::optional<size_t>> visited;
    GraphBuilVertexWaveNumbers<true, true>(graph, visited, from);
    std::vector<size_t> path;
    if (!FindPathByWaveNumbers(graph, visited, path, from, to)) {
        *output << "Can't find path from " << from << " to " << to << '\n';
        return;
    }

    // Print path
    *output << "\n\nPath: ";
    for (size_t i = 0; i < path.size(); ++i) {
        const size_t vertex = path[i];
        *output << vertex;
        if (i < path.size() - 1) {
            *output << " -> ";
        }
    }

    // Print vertex wave numbers
    *output << "\n\nWave numbers for vertices:\n";
    for (size_t i = 0; i < visited.size(); ++i) {
        *output << "   " << i << ": ";
        if (visited[i].has_value()) {
            *output << *visited[i];
        }
        else {
            *output << "path not found";
        }
        *output << '\n';
    }
}

void LeeAlgorithmTest() {
    const std::string sample_input =
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

    LeeAlgorithmApp(std::stringstream(sample_input), &std::cout);
}

int main() {

    try
    {
        LeeAlgorithmTest();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
