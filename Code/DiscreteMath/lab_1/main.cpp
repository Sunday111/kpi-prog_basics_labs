#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

constexpr bool UseExceptions = true;

static constexpr std::string_view MustReadValueMessage = "Reading value from input stream";

inline void Assert(const bool value, const std::string_view message)
{
    if(!value)
    {
        if constexpr (UseExceptions)
		{
			static constexpr std::string_view AssertionFailedMessage = "Assertion failed: ";
			std::stringstream sstream;
			sstream << AssertionFailedMessage << message;
            throw std::runtime_error(sstream.str());
        }
        else
        {
            assert(false);
        }
    }
}

template<typename T>
inline bool TryReadValue(std::istream& input, T& value) {
    return !(!(input >> value));
}


template<typename T>
inline void MustReadValue(std::istream& input, T& value, const std::string_view message = MustReadValueMessage) {
    Assert(TryReadValue(input, value), message);
}

template<typename T>
inline T MustReadValue(std::istream& input, const std::string_view message = MustReadValueMessage) {
    T result;
    MustReadValue(input, result, message);
    return result;
}

template<typename T, bool oriented = true, bool allowLoops = false>
class GraphAdjacencyMatrix
{
public:
	GraphAdjacencyMatrix(GraphAdjacencyMatrix&&) = default;
	GraphAdjacencyMatrix(const GraphAdjacencyMatrix&) = delete;

    GraphAdjacencyMatrix(size_t verticesCount) :
        m_verticesCount(verticesCount),
        m_edgeFlags(EdgesCountByVerticesCount(verticesCount), false)
	{
		Assert(verticesCount > 0, "vertices count > 2");
    }

    void SetEdge(size_t i, size_t j, T value) {
		ValidateIndices(i, j);
        const size_t idx = EdgeIndex(i, j, m_verticesCount);
        m_edgeFlags[idx] = value;
    }

	[[nodiscard]] decltype(auto) GetEdge(size_t i, size_t j) const {
		ValidateIndices(i, j);
        const size_t idx = EdgeIndex(i, j, m_verticesCount);
        return m_edgeFlags[idx];
    }

	[[nodiscard]] size_t GetVerticesCount() const {
		return m_verticesCount;
	}

	[[nodiscard]] size_t GetMaxVertexConnections() const {
		if constexpr (oriented || loopsAllowed) {
			return m_verticesCount;
		} else {
			return m_verticesCount - 1;
		}
	}

	GraphAdjacencyMatrix& operator= (GraphAdjacencyMatrix&&) = default;
	GraphAdjacencyMatrix& operator= (const GraphAdjacencyMatrix&) = delete;

private:
	inline void ValidateIndices(size_t i, size_t j) const {
		ValidateIndex(i);
		ValidateIndex(j);
		if constexpr (!oriented && !allowLoops) {
			Assert(i != j, "Loops are not allowed for unoriented graph");
		}
	}

	inline void ValidateIndex(size_t index) const {
		Assert(index < m_verticesCount, "index < vertices count");
	}

    [[nodiscard]] static inline constexpr size_t EdgesCountByVerticesCount(const size_t verticesCount) noexcept {
		if constexpr (oriented) {
			return verticesCount * verticesCount;
		} else {
			if constexpr (allowLoops) {
				return verticesCount * (verticesCount + 1) / 2;
			} else {
				return verticesCount * (verticesCount - 1) / 2;
			}
		}
    }

    [[nodiscard]] static inline constexpr std::pair<size_t, size_t> CorrectIndices(const size_t i, const size_t j) noexcept {
		if constexpr (oriented) {
			return std::pair(i, j);
		} else {
			return i > j ? std::pair(j, i) : std::pair(i, j);
		}
    }

    [[nodiscard]] static inline constexpr size_t EdgeIndex(const size_t i_, const size_t j_, const size_t v) noexcept {
		if constexpr (oriented) {
			return v * i_ + j_;
		} else {
			const auto[i, j] = CorrectIndices(i_, j_);
			return EdgesCountByVerticesCount(j) + i;
		}
    }

private:
    size_t m_verticesCount;
    std::vector<T> m_edgeFlags;
};

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

template<bool oriented = false, bool allowLoops = false>
GraphAdjacencyMatrix<bool, oriented, allowLoops> ReadGraph(std::istream& input, std::ostream* output) {
	if (output) {
		*output << "Enter vertices count: ";
	}

	const size_t verticesCount = MustReadValue<size_t>(input, "Read vertices count");

	GraphAdjacencyMatrix<bool, oriented, allowLoops> adjacencyMatrix(verticesCount);
	adjacencyMatrix.SetEdge(0, 1, true);
	bool val = adjacencyMatrix.GetEdge(1, 0);

	if (output) {
		if constexpr (oriented) {
			*output << "Enter full adjacency matrix \n";
		} else {
			if constexpr (allowLoops) {
				*output << "Now you need to enter bottom triangle of adjacency matrix and main diagonal. \n";
			} else {
				*output << "Now you need to enter bottom triangle of adjacency matrix without main diagonal. \n";
			}
		}
	}

	auto printInviteForVertex = [&](size_t vertex) {
		if (output) {
			*output << std::setw(10) << vertex << ": ";
		}
	};

	size_t i = (!oriented && !allowLoops) ? 1 : 0;
	size_t j = 0;

	printInviteForVertex(i);

	auto needSwitchRow = [](size_t i, size_t j, size_t n) {
		if constexpr (oriented) {
			return j >= n;
		} else {
			if constexpr (allowLoops) {
				return  j > i;
			} else {
				return  j >= i;
			}
		}
	};

	do
	{
		size_t edgeValue = MustReadValue<size_t>(input, "Read edge between vertices");
		adjacencyMatrix.SetEdge(i, j, edgeValue);

		++j;
		if (needSwitchRow(i, j, verticesCount)) {
			++i;
			if (i < verticesCount) {
				printInviteForVertex(i);
			}
			j = 0;
		}
	} while (i < verticesCount);

	return adjacencyMatrix;
}

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
void ComputeVertexDegree(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>& g, std::istream& input, std::ostream* output)
{
	const size_t n = g.GetVerticesCount();
	for (size_t i = 0; i < n; ++i) {
		if (output) {
			*output << "Vertex " << i << " has edge with " << GetVertexDegree(g, i) << " vertices\n";
		}
	}
}

template<bool oriented, bool loopsAllowed>
void FindIsolatedVertices(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>&, std::istream& input, std::ostream* output)
{

}

template<bool oriented, bool loopsAllowed>
void FindLeafs(const GraphAdjacencyMatrix<bool, oriented, loopsAllowed>&, std::istream& input, std::ostream* output)
{

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
