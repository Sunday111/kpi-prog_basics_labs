#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>

#include "graph_lib/GraphMaxEdgeDisjointPaths.h"
#include "graph_lib/ReadGraph.h"

void MaxGraphFlowApp(std::istream& input, std::ostream* output) {
    auto graph = ReadGraph<size_t, true, true>(input, output);
    const size_t from = MustReadValue<size_t>(input, "Enter source vertex index: ");
    const size_t to = MustReadValue<size_t>(input, "Enter sink vertex index: ");
    const auto maxFlow = GraphMaxFlow(graph, from, to);
    *output << "\nMax flow: " << maxFlow << '\n';
}

void MaxGraphFlowAppTest() {
    const std::string_view sample_input =
        /* vertices count */"5 \n"
        /*     0 1 2 3 4  */
        /*0*/ "0 2 0 0 5 \n"
        /*1*/ "0 0 3 0 0 \n"
        /*2*/ "0 0 0 3 2 \n"
        /*3*/ "3 3 0 0 3 \n"
        /*4*/ "0 0 0 0 0 \n"
        /* start vertex */"0 \n"
        /*   end vertex */"4 \n";
    MaxGraphFlowApp(std::stringstream(std::string(sample_input)), &std::cout);
}

void MaxEdgeDisjointPathsApp(std::istream& input, std::ostream* output) {
    auto graph = ReadGraph<size_t, true, true>(input, output);
    const size_t from = MustReadValue<size_t>(input, "Enter source vertex index: ");
    const size_t to = MustReadValue<size_t>(input, "Enter sink vertex index: ");
    const auto maxPaths = GraphMaxEdgeDisjointPaths(graph, from, to);
    *output << "Max edge disjoint paths: " << maxPaths << '\n';
}

void MaxEdgeDisjointPathsTest() {
    const std::string_view sample_input =
        /* vertices count */"5 \n"
        /*     0 1 2 3 4 */
        /*0*/ "0 2 0 0 5 \n"
        /*1*/ "0 0 3 0 0 \n"
        /*2*/ "0 0 0 3 2 \n"
        /*3*/ "3 3 0 0 3 \n"
        /*4*/ "0 0 0 0 0 \n"
        /* start vertex */"0 \n"
        /*   end vertex */"4 \n";

    MaxEdgeDisjointPathsApp(std::stringstream(std::string(sample_input)), &std::cout);
}

int main() {
    try
    {
        MaxGraphFlowAppTest();
        //MaxEdgeDisjointPathsTest();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
