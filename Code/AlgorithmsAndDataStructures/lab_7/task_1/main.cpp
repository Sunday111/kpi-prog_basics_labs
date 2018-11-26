#include <climits>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

#include "BinaryTree/BinarySearchTree.h"
#include "BinaryTree/RedBlackTree.h"
#include "BinaryTree/PrintTree.h"

class Multistream
{
public:
    template<typename... Args>
    void Print(Args&&... args) {
        for (std::ostream* pStream : m_streams) {
            (*pStream << ... << args);
        }
    }

    void AddStream(std::ostream* pStream) {
        m_streams.push_back(pStream);
    }

    template<typename... Args>
    void PrintLine(Args&&... args) {
        Print(std::forward<Args>(args)..., '\n');
    }

private:
    std::vector<std::ostream*> m_streams;
};

#ifdef _DEBUG
constexpr bool validateTree = true;
#else
constexpr bool validateTree = false;
#endif

template<typename T, typename Generator, typename Distribution>
void GenerateRandomVector(std::vector<T>& values, size_t valuesCount, bool unique, bool sorted, Generator&& gen, Distribution& distr) {
    values.clear();
    values.reserve(valuesCount);
    for (size_t i = 0; i < valuesCount; ++i) {
        auto it = values.begin();
        T newValue;
        do
        {
            newValue = distr(gen);
            it = std::lower_bound(values.begin(), values.end(), newValue);
        } while (!unique || !(it == values.end() || *it != newValue));

        values.insert(it, newValue);
    }

    if (!sorted) {
        std::shuffle(values.begin(), values.end(), gen);
    }
}

void VisualizeInsert(Multistream& output) {
    using T = size_t;
    using Tree = BinarySearchTree<T>;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> valueDistribution(0, 10);

    constexpr size_t valuesCount = 10;
    std::vector<T> values;

    BinarySearchTree<T> bst;
    RedBlackTree<T> rbt;
    GenerateRandomVector<T>(values, valuesCount, true, false, gen, valueDistribution);
    for (size_t i = 0; i < valuesCount; ++i) {
        system("cls");
        rbt.Emplace(values[i]);
        rbt.Print(std::cout);
    }
}

void Main(Multistream& output) {
    using T = size_t;
    using Tree = BinarySearchTree<T>;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> valueDistribution(
        std::numeric_limits<T>::lowest() / 2,
        std::numeric_limits<T>::max() / 2
    );

    constexpr size_t valuesCount = 60000;
    std::vector<T> values;
    
    auto generateUniqueValues = [&]() {
        GenerateRandomVector<T>(values, valuesCount, true, false, gen, valueDistribution);
    };

    generateUniqueValues();

    BinarySearchTree<T> bst;
    RedBlackTree<T> rbt;

    auto emplace = [](auto& tree, T value) {
        bool result = tree.Emplace(value);
        if constexpr (validateTree) {
            if (!result) {
                throw std::runtime_error("Failed to emplace");
            }
        }
    };

    auto find = [](auto& tree, T value) {
        bool result = tree.HasValue(value);
        if constexpr (validateTree) {
            if (!result) {
                throw std::runtime_error("Value not found");
            }
        }
    };

    for (size_t i = 0; i < values.size(); ++i) {
        emplace(bst, values[i]);
        find(bst, values[i]);

        emplace(rbt, values[i]);
        find(rbt, values[i]);
    }
}

int main(int, char**) {
    Multistream output;
    output.AddStream(&std::cout);

    std::ofstream logstream("log.txt");
    output.AddStream(&logstream);

    try
    {
        VisualizeInsert(output);
        //Main(output);
    }
    catch (const std::exception& ex)
    {
        output.PrintLine("Exception: ", ex.what());
        return 1;
    }

    return 0;
}