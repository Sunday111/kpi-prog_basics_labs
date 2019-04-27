#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <string_view>
#include <vector>

template<typename Vertex>
class Graph
{
public:
    void SetVerticesCount(const size_t verticesCount) {
		m_verticesCount = verticesCount;

		m_vertices.resize(0);
		m_vertices.resize(m_verticesCount);

		m_edgeTable.resize(0);
		const size_t maxEdgesCount = verticesCount * verticesCount;
		m_edgeTable.resize(maxEdgesCount);
    }

	Vertex& GetVertex(size_t index) {
		return m_vertices[index];
	}

	const Vertex& GetVertex(size_t index) const {
		return m_vertices[index];
	}

	void SetEdge(size_t i, size_t j, bool value) {
		const size_t edgeIndex = AbsoluteIndex(i, j);
		m_edgeTable[edgeIndex] = value;
	}

	bool HasEdge(size_t i, size_t j) const {
		const size_t edgeIndex = AbsoluteIndex(i, j);
		return m_edgeTable[edgeIndex];
	}

	void DepthFirstTraverse(size_t root, std::function<void(Vertex&)> callback) {
		std::vector<bool> visited(m_verticesCount, false);
		std::vector<size_t> queue;

		queue.push_back(root);

		while (!queue.empty()) {
			const size_t i = queue.back();
			queue.pop_back();
			if (!visited[i]) {
				visited[i] = true;
				callback(m_vertices[i]);
				for (size_t j = 0; j < m_verticesCount; ++j) {
					if (!visited[j] && HasEdge(i, j)) {
						queue.push_back(j);
					}
				}
			}
		}
	}

	void DepthFirstTraverse(size_t root, std::function<void(const Vertex&)> callback) const {
		auto this_ = const_cast<Graph*>(this);
		this_->DepthFirstTraverse(root, [&](Vertex& vertex) {
			callback(vertex);
		});
	}

protected:
	size_t AbsoluteIndex(size_t i, size_t j) const {
		return i * m_verticesCount + j;
	}

private:
	size_t m_verticesCount = 0;
	std::vector<bool> m_edgeTable;
	std::vector<Vertex> m_vertices;
};

template<typename T>
class AddValueInScope
{
public:
	explicit AddValueInScope(T& value, T add) :
		m_add(add),
		m_reference(value)
	{
		m_reference += m_add;
	}

	~AddValueInScope() {
		m_reference -= m_add;
	}

private:
	T m_add;
	T& m_reference;
};

int main() {
    using Vertex = size_t;
	using Clock = std::chrono::high_resolution_clock;
	using Duration = long double;
	using DurationScale = std::chrono::nanoseconds;
	constexpr auto durationString = "ns";
	constexpr size_t edgeProbability = 50;

    std::random_device randomDevice;
    std::mt19937_64 randomGenerator(randomDevice());
	std::uniform_int_distribution<size_t> valueDistribution(std::numeric_limits<size_t>::lowest(), std::numeric_limits<size_t>::max());
	std::uniform_int_distribution<size_t> edgeDistribution(1, 100);

	std::ostream& logstream = std::cout;
	size_t logOffset = 0;
	char offsetSymbol = ' ';

	logstream << std::fixed;

	auto print = [&](auto... values) {
		std::ostream_iterator<std::decay_t<decltype(offsetSymbol)>> streamIterator(logstream, "");
		std::fill_n(streamIterator, logOffset, offsetSymbol);
		(logstream << ... << values);
	};

	auto println = [&print](auto... values) {
		print(values...);
		print('\n');
	};

	auto getDuration = [](auto fn) {
		auto t1 = Clock::now();
		fn();
		auto t2 = Clock::now();
		auto dt = std::chrono::duration_cast<DurationScale>(t2 - t1);
		return static_cast<Duration>(dt.count());
	};

	auto runTask = [&](std::string_view title, auto task) {
		println("Begin task: ", title);
		Duration duration;
		{
			AddValueInScope<size_t> scopeAdd(logOffset, 3);
			duration = getDuration(task);
		}
		println("End task: ", title, " (", duration, durationString, ")");
	};

	auto generateVertex = [&]() {
		return valueDistribution(randomGenerator);
	};

	auto generateEdge = [&]() {
		const size_t val = edgeDistribution(randomGenerator);
		return val <= edgeProbability;
	};

	constexpr size_t verticesCount = 5;
	Graph<Vertex> graph;

	runTask("Generate graph", [&]() {
		runTask("Generate graph vertices", [&]() {
			graph.SetVerticesCount(verticesCount);
			for (size_t i = 0; i < verticesCount; ++i) {
				graph.GetVertex(i) = generateVertex();
			}
		});

		runTask("Generate graph edges", [&]() {
			graph.SetVerticesCount(verticesCount);
			for (size_t i = 0; i < verticesCount; ++i) {
				for (size_t j = 0; j < verticesCount; ++j) {
					graph.SetEdge(i, j, generateEdge());
				}
			}
		});

		runTask("Depth first traverse", [&]() {
			volatile size_t sum = 0;
			graph.DepthFirstTraverse(0, [&](const Vertex& vertex) {
				sum += vertex;
			});
		});
	});

    return 0;
}
