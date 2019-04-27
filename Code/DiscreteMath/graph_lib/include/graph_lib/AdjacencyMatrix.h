#pragma once

#include <vector>
#include "ErrorHandling.h"

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
