#pragma once

#include <functional>
#include <sstream>
#include <vector>
#include "BinaryTree.h"

template<typename T>
void PrintTree(
    BinaryTreeLeafPtr<T>& root, std::ostream& output,
    std::function<void(const BinaryTreeLeaf<T>&, std::ostream&)> printLeafData)
{
    using Leaf = BinaryTreeLeaf<T>;
    std::vector<Leaf*> queue;
    std::vector<Leaf*> backlog;

    auto forEachLevel = [&](auto& callback) {
        queue.clear();
        backlog.clear();
        backlog.push_back(root.get());
        size_t depth = 0;
        while (!backlog.empty()) {
            std::swap(queue, backlog);
            //std::reverse(queue.begin(), queue.end());
            callback(depth++, queue);
            bool allNull = true;
            for (auto p : queue) {
                if (p != nullptr) {
                    backlog.push_back(p->GetLink(Direction::Left).get());
                    backlog.push_back(p->GetLink(Direction::Right).get());
                    if (backlog[backlog.size() - 1] || backlog[backlog.size() - 2]) {
                        allNull = false;
                    }
                }
                else {
                    backlog.push_back(nullptr);
                    backlog.push_back(nullptr);
                }
            }
            if (allNull) {
                break;
            }
            queue.clear();
        }
    };

    size_t maxElementsInRow = 0;
    size_t maxDepth = 0;
    size_t maxValuePrintLength = 0;

    auto printLeaf = [&printLeafData](Leaf* leaf, std::ostream& out) {
        if (leaf) {
            out << ' ';
            printLeafData(*leaf, out);
            out << ' ';
        }
        else {
            out << " null ";
        }
    };

    auto get_leaf_print_length = [&printLeaf](Leaf* leaf) {
        std::stringstream stream;
        printLeaf(leaf, stream);
        return stream.str().size();
    };

    forEachLevel([&](size_t depth, std::vector<Leaf*>& leafs) {
        maxElementsInRow = std::max(leafs.size(), maxElementsInRow);
        maxDepth = depth;
        for (auto leaf : leafs) {
            maxValuePrintLength = std::max(maxValuePrintLength, get_leaf_print_length(leaf));
        }
    });

    auto pow = [](size_t val, size_t power) {
        size_t result = 1;
        for (size_t i = 0; i < power; ++i) {
            result *= val;
        }
        return result;
    };

    size_t nodePrintLength = maxValuePrintLength + 2;
    size_t textMaxWidth = nodePrintLength * maxElementsInRow;

    constexpr char leftCorner = 201ui8;
    constexpr char line = 205ui8;
    constexpr char rightCorner = 187ui8;
    constexpr char filler = ' ';

    auto draw_n = [&](char s, size_t n) {
        std::fill_n(std::ostream_iterator<char>(output), n, s);
    };

    forEachLevel([&](size_t depth, std::vector<Leaf*>& leafs) {
        const size_t segmentsCount = pow(2, depth);
        const size_t segmentSize = textMaxWidth / segmentsCount;
        const size_t desiredFreeSpace = segmentSize / 4;

        for (Leaf* leaf : leafs) {
            const size_t leaf_print_length = get_leaf_print_length(leaf);
            const size_t freeSpace = segmentSize - leaf_print_length;
            const size_t freeLeft = freeSpace / 2;
            const size_t freeRight = freeSpace - freeLeft;

            if (freeLeft > 0) {
                size_t arrowSize = 0;
                if (depth < maxDepth) {
                    if (freeLeft <= desiredFreeSpace) {
                        arrowSize = 1;
                    }
                    else {
                        arrowSize = freeLeft - desiredFreeSpace;
                    }
                }
                const size_t fillerSize = freeLeft - arrowSize;

                draw_n(filler, fillerSize);

                if (arrowSize > 0) {
                    draw_n(leftCorner, 1);
                    draw_n(line, arrowSize - 1);
                }
            }

            printLeaf(leaf, output);

            if (freeRight > 0) {
                size_t arrowSize = 0;
                if (depth < maxDepth) {
                    if (freeRight <= desiredFreeSpace) {
                        arrowSize = 1;
                    }
                    else {
                        arrowSize = freeRight - desiredFreeSpace;
                    }
                }
                const size_t fillerSize = freeRight - arrowSize;

                if (arrowSize > 0) {
                    draw_n(line, arrowSize - 1);
                    draw_n(rightCorner, 1);
                }
                draw_n(filler, fillerSize);
            }
        }

        output << '\n';
    });
}