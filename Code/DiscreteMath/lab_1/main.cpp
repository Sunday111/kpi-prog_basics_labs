#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

#include "graph_lib/GetVertexDegree.h"
#include "graph_lib/ReadGraph.h"

struct AppArgs
{
	bool oriented = false;
	bool loopsAllowed = false;

	static AppArgs ReadFromInput(std::istream& input, std::ostream* output) {
		AppArgs result;

		constexpr std::string_view boolHintMessage = "(Enter 1 if true and 0 if false)";
		if (output) *output << "Graph is oriented" << boolHintMessage << ": ";
		result.oriented = MustReadValue<int>(input);

		if (output) *output << "Graph can have loops" << boolHintMessage << ": ";
		result.loopsAllowed = MustReadValue<int>(input);

		return result;
	}
};

template<bool oriented, bool loopsAllowed>
void ComputeVertexDegree(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>& g, std::istream& input, std::ostream* output) {
	const size_t n = g.GetVerticesCount();
	for (size_t i = 0; i < n; ++i) {
		const size_t degree = GetVertexDegree(g, i);
		if (output) {
			*output << "Vertex " << i << " has edge with " << degree << " vertices\n";
		}
	}
}

template<bool oriented, bool loopsAllowed>
void FindIsolatedVertices(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>& g, std::istream& input, std::ostream* output) {
	const size_t n = g.GetVerticesCount();
	for (size_t i = 0; i < n; ++i) {
		const size_t degree = GetVertexDegree(g, i);
		if (degree == 0 && output) {
			*output << "Vertex " << i << " is isolated \n";
		}
	}
}

template<bool oriented, bool loopsAllowed>
void FindLeafs(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>& g, std::istream& input, std::ostream* output) {
	const size_t n = g.GetVerticesCount();
	for (size_t i = 0; i < n; ++i) {
		const size_t degree = GetVertexDegree(g, i);
		if (degree == 1 && output) {
			*output << "Vertex " << i << " is a leaf \n";
		}
	}
}

template<bool oriented, bool loopsAllowed>
void Main(std::istream& input, std::ostream* output) {
	auto graphAdjacencyMatrix = ReadGraph<oriented, loopsAllowed>(input, output);

	using CommandFn = std::function<void(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>&, std::istream&, std::ostream*)>;
	std::vector<std::tuple<std::string_view, CommandFn>> commands
	{
		{ "Find vertex degree (number of connected vertices with specified vertex)", ComputeVertexDegree<oriented, loopsAllowed> },
		{ "Find isolated vertices (vertices without connections, degree is 0)", FindIsolatedVertices<oriented, loopsAllowed> },
		{ "Find leafs (leafs are vertices with one connection, degree is 1)", FindLeafs<oriented, loopsAllowed> },
	};

	auto getCommandIndex = [&]() {
		if (output)
		{
			*output << "List of commands: \n";
			for (size_t i = 0; i < commands.size(); ++i) {
				auto& command = commands[i];
				*output << std::setw(3) << i << ": " << std::get<std::string_view>(command) << '\n';
			}

			*output << '\n';
			*output << "Enter command index or '-1' to stop: ";
		}

		return MustReadValue<int>(input, "Read command index");
	};

	int ci = -1;
	while ((ci = getCommandIndex()) != -1) {
		if(ci < 0 || ci >= commands.size()) {
			continue;
		}

		auto& cmd = commands[ci];
		std::get<CommandFn>(cmd)(graphAdjacencyMatrix, input, output);
	}
}

int main() {
    std::istream& input = std::cin;
    std::ostream* output = &std::cout;

    try
    {
		const AppArgs args = AppArgs::ReadFromInput(input, output);

		using MainFn = void(*)(std::istream&, std::ostream*);

		MainFn mainFunction = nullptr;

		if (args.oriented && args.loopsAllowed) {
			mainFunction = Main<true, true>;
		} else if (!args.oriented && args.loopsAllowed) {
			mainFunction = Main<false, true>;
		} else if (args.oriented && !args.loopsAllowed) {
			mainFunction = Main<true, false>;
		} else if (!args.oriented && !args.loopsAllowed) {
			mainFunction = Main<false, false>;
		}

		if (mainFunction) {
			mainFunction(input, output);
		}
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
