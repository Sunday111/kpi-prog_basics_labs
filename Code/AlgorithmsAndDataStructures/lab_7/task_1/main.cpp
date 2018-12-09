#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

#include "BinaryTree/BinarySearchTree.h"
#include "BinaryTree/RedBlackTree.h"
#include "BinaryTree/PrintTree.h"
#include "SystemTimer.h"

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
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::nanoseconds;
    using DurationRep = long double;
    const size_t stepsCount = 100;
    const size_t valuesCount = 100000;
    const size_t stepSize = valuesCount / stepsCount;
    const size_t passesCount = 20;
    const size_t totalStepsCount = stepsCount * passesCount;
    const char splitCharacter = ';';
    const bool sortedInput = false;
    static_assert((valuesCount % stepsCount) == 0);


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> valueDistribution(
        std::numeric_limits<T>::lowest() / 2,
        std::numeric_limits<T>::max() / 2
    );

    auto get_duration = [&](auto& fn) {
        const auto t1 = Clock::now();
        fn();
        const auto t2 = Clock::now();
        const auto dt = std::chrono::duration_cast<Duration>(t2 - t1);
        return static_cast<DurationRep>(dt.count());
    };

    //SystemTimer tm;
    //auto get_duration = [&](auto& fn) {
    //    const auto t1 = tm.Now();
    //    fn();
    //    const auto t2 = tm.Now();
    //    const auto dt = (t2 - t1) * 1000000000;
    //    return dt;
    //};

    auto emplace = [](auto& tree, T value) {
        if constexpr (validateTree) {
            const bool alreadyHasValue = tree.HasValue(value);
            if (alreadyHasValue) {
                throw std::runtime_error("Tree already has value");
            }
        }
        const bool result = tree.Emplace(value);
        if constexpr (validateTree) {
            if (!result) {
                throw std::runtime_error("Failed to emplace");
            }
        }
    };

    auto find = [](auto& tree, T value) {
        const volatile bool result = tree.HasValue(value);
        if constexpr (validateTree) {
            if (!result) {
                throw std::runtime_error("Value not found");
            }
        }
    };

    std::vector<T> passValues;

    auto stepMeanDuation = [&](auto& tree, auto& methodWrapper, size_t step) {
        const auto stepDuration = get_duration([&]() {
            const size_t begin = step * stepSize;
            const size_t end = begin + stepSize;
            for (size_t i = begin; i < end; ++i) {
                methodWrapper(tree, passValues[i]);
            }
        });

        return stepDuration / stepSize;
    };

    struct CollectionInfo {
        std::vector<DurationRep> emplaceTime;
        std::vector<DurationRep> findTime;

    };

    struct PassInfo {
        CollectionInfo binarySearchTree;
        CollectionInfo redBlackTree;
    };

    std::vector<PassInfo> passesInfo;

    for (size_t pass = 0; pass < passesCount; ++pass) {
        PassInfo passInfo;
        BinarySearchTree<T> bst;
        RedBlackTree<T> rbt;

        GenerateRandomVector<T>(passValues, valuesCount, true, sortedInput, gen, valueDistribution);

        for (size_t step = 0; step < stepsCount; ++step) {
            auto bstEmplace = stepMeanDuation(bst, emplace, step);
            passInfo.binarySearchTree.emplaceTime.push_back(bstEmplace);

            auto bstFind = stepMeanDuation(bst, find, step);
            passInfo.binarySearchTree.findTime.push_back(bstFind);

            auto rbtEmplace = stepMeanDuation(rbt, emplace, step);
            passInfo.redBlackTree.emplaceTime.push_back(rbtEmplace);

            auto rbtFind = stepMeanDuation(rbt, find, step);
            passInfo.redBlackTree.findTime.push_back(rbtFind);
        }

        passesInfo.push_back(std::move(passInfo));
    }

    PassInfo meanPass;

    // Compute mean pass
    {
        auto stepMeanDurationsAcrossPasses = [&](CollectionInfo PassInfo::*collection, std::vector<DurationRep> CollectionInfo::*operation, size_t step) {
            DurationRep meanDuration{};
            for (auto& passInfo : passesInfo) {
                meanDuration += ((passInfo.*collection).*operation)[step];
            }

            ((meanPass.*collection).*operation).push_back(meanDuration / passesInfo.size());
        };

        for (size_t step = 0; step < stepsCount; ++step) {
            stepMeanDurationsAcrossPasses(&PassInfo::binarySearchTree, &CollectionInfo::emplaceTime, step);
            stepMeanDurationsAcrossPasses(&PassInfo::binarySearchTree, &CollectionInfo::findTime, step);
            stepMeanDurationsAcrossPasses(&PassInfo::redBlackTree, &CollectionInfo::emplaceTime, step);
            stepMeanDurationsAcrossPasses(&PassInfo::redBlackTree, &CollectionInfo::findTime, step);
        }
    }

    output.PrintLine("Emplace time elements count");
    output.PrintLine("Elements count", splitCharacter, "Binary search tree", splitCharacter, "Red-Black tree");
    for (size_t step = 0; step < stepsCount; ++step) {
        output.Print(step * stepSize);
        output.Print(splitCharacter);
        output.Print(meanPass.binarySearchTree.emplaceTime[step]);
        output.Print(splitCharacter);
        output.Print(meanPass.redBlackTree.emplaceTime[step]);
        output.PrintLine();
    }

    output.PrintLine();
    output.PrintLine("Find time elements count");
    output.PrintLine("Elements count", splitCharacter, "Binary search tree", splitCharacter, "Red-Black tree");
    for (size_t step = 0; step < stepsCount; ++step) {
        output.Print(step * stepSize);
        output.Print(splitCharacter);
        output.Print(meanPass.binarySearchTree.findTime[step]);
        output.Print(splitCharacter);
        output.Print(meanPass.redBlackTree.findTime[step]);
        output.PrintLine();
    }
}

int main(int, char**) {
    Multistream output;
    output.AddStream(&std::cout);

    std::ofstream logstream("log.txt");
    output.AddStream(&logstream);

    try
    {
        //VisualizeInsert(output);
        Main(output);
    }
    catch (const std::exception& ex)
    {
        output.PrintLine("Exception: ", ex.what());
        return 1;
    }

    return 0;
}