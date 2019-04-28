#pragma once

#include <ostream>
#include <iomanip>
#include <istream>

#include "AdjacencyMatrix.h"
#include "InputUtility.h"

template<typename T, bool oriented = false, bool allowLoops = false>
GraphAdjacencyMatrix<T, oriented, allowLoops> ReadGraph(std::istream& input, std::ostream* output) {
	if (output) {
		*output << "Enter vertices count: ";
	}

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

    const size_t verticesCount = MustReadValue<size_t>(input, "Read vertices count");
    GraphAdjacencyMatrix<T, oriented, allowLoops> adjacencyMatrix(verticesCount);

	do
	{
		T edgeValue = MustReadValue<T>(input, "Read edge between vertices");
		adjacencyMatrix.SetEdge(i, j, std::move(edgeValue));

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