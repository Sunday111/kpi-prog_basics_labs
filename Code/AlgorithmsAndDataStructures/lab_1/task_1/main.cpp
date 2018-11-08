#include "Array/Array.h"
#include "LinkedList/LinkedList.h"
#include "Thread/ThreadPool.h"
#include <cassert>
#include <random>
#include <vector>
#include <chrono>
#include <iostream>
#include <string>

template<template<typename> typename Layout>
void StackTests() {
    constexpr size_t commandsCount = 100000;
    constexpr int minValue = -100000;
    constexpr int maxValue = 100000;

    using value_type = int;
    Layout<value_type> stack;
    std::vector<value_type> reference;

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> commandDistribution(0, 1);
    std::uniform_int_distribution<> valueDistribution(minValue, maxValue);

    auto compareWithReference = [&]() {
        auto a = stack.GetIterator();
        auto b = reference.begin();

        while (a.HasValue()) {
            assert(b != reference.end());
            assert(a.Value() == *b);
            a.Advance();
            ++b;
        }
    };

    for (size_t i = 0; i < commandsCount; ++i) {
        if (commandDistribution(gen)) {
            auto value = valueDistribution(gen);
            stack.EmplaceBack(value);
            reference.emplace_back(value);
        } else {
            stack.PopBack();
            if (!reference.empty()) {
                reference.pop_back();
            }
        }
        compareWithReference();
    }
}

template<typename TimeScale = std::chrono::milliseconds, typename F>
auto GetProcessDuration(F&& f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<TimeScale>(end - start);
}

template
<
    typename OnePassState,
    typename TimeScale = std::chrono::milliseconds,
    typename DurationType = long double
>
class OperationProfiler
{ 
public:
    using UserOperation = std::function<void(OnePassState&)>;

    OperationProfiler(size_t desiredThreadsCount = 0) :
        m_threadPool(desiredThreadsCount)
    {}

    struct Operation
    {
        std::string name;
        size_t repeatsCount = 1;
        UserOperation function;
    };

    void SetPassesCount(size_t passesCount) {
        m_passesCount = passesCount;
    }

    void AddOperation(std::string name, UserOperation fn) {
        Operation operation;
        operation.name = std::move(name);
        operation.function = std::move(fn);
        m_algorithm.push_back(std::move(operation));
    }

    struct OperationInfo
    {
        std::string name;
        DurationType min = 0;
        DurationType max = 0;
        DurationType mean = 0;
    };

    std::vector<OperationInfo> DoProfiling() {
        m_threadPool.Start();
        for (size_t i = 0; i < m_passesCount; ++i) {
            m_threadPool.AddJob([&]() {
                OnePassState state;
                std::vector<TimeScale> result;
                for (Operation& operation : m_algorithm) {
                    TimeScale duration = GetProcessDuration<TimeScale>([&]() {
                        operation.function(state);
                    });
                    result.push_back(duration);
                }

                return result;
            });
        }
        m_threadPool.Stop();

        std::vector<OperationInfo> operationsInfo;
        operationsInfo.reserve(m_algorithm.size());

        for (size_t i = 0; i < m_algorithm.size(); ++i) {
            operationsInfo.emplace_back();
            OperationInfo& operationInfo = operationsInfo[i];
            operationInfo.name = m_algorithm[i].name;
            operationInfo.min = std::numeric_limits<DurationType>::max();
            operationInfo.max = std::numeric_limits<DurationType>::lowest();
            operationInfo.mean = 0;
        }

        m_threadPool.ForEachResult([&](std::vector<TimeScale>& result) {
            for (size_t i = 0; i < result.size(); ++i) {
                OperationInfo& operationInfo = operationsInfo[i];
                auto duration = static_cast<DurationType>(result[i].count());
                operationInfo.min = std::min(operationInfo.min, duration);
                operationInfo.max = std::max(operationInfo.max, duration);
                operationInfo.mean += duration / m_passesCount;
            }
        });

        return operationsInfo;
    }

private:
    size_t m_passesCount = 1;
    ThreadPool<std::vector<TimeScale>()> m_threadPool;
    std::vector<Operation> m_algorithm;
};

struct NoCapacityPolicy
{
    std::size_t GetNewCapacity(std::size_t requestedCapacity, std::size_t) {
        return requestedCapacity;
    }
};

template<typename Element, size_t operations, size_t passes, size_t threads>
struct StackProfiler
{
    template<template<typename> typename Layout>
    static void StackPerfomance(std::string title) {
        using StateType = Layout<Element>;
        OperationProfiler<StateType> profiler(threads);
        profiler.SetPassesCount(passes);
        profiler.AddOperation("Pushes", [&](StateType& state) {
            for (size_t i = 0; i < operations; ++i) {
                state.EmplaceBack(10);
            }
        });
        profiler.AddOperation("Pops", [&](StateType& state) {
            for (size_t i = 0; i < operations; ++i) {
                state.PopBack();
            }
        });

        auto operationsInfo = profiler.DoProfiling();
        std::cout << title << '\n';
        for (auto& operationInfo : operationsInfo) {
            std::cout << '\t' << operationInfo.name << '\n';
            std::cout << "\t\tmin:  " << operationInfo.min << "ms\n";
            std::cout << "\t\tmean: " << operationInfo.mean << "ms\n";
            std::cout << "\t\tmax:  " << operationInfo.max << "ms\n";
        }
        std::cout << "\n";
    }
};

template<typename T> using LinkedList_ = LinkedList<T, false, false>;
template<typename T> using DoublyLinkedList = LinkedList<T, true, false>;
template<typename T> using LinkedList_StoredTail = LinkedList<T, false, true>;
template<typename T> using DoublyLinkedList_StoredTail = LinkedList<T, true, true>;
template<typename T> using StackArray = Array<T, NoCapacityPolicy>;
template<typename T> using StackArray_Capacity = Array<T, DefaultCapacityPolicy>;

int main() {
    constexpr size_t operations = 100000;
    constexpr size_t passes = 100;
    constexpr size_t threads = 1;
    using Profiler = StackProfiler<int, operations, passes, threads>;
    Profiler::StackPerfomance<LinkedList_>("Linked list");
    Profiler::StackPerfomance<DoublyLinkedList>("Doubly linked list");
    Profiler::StackPerfomance<LinkedList_StoredTail>("Linked list with pointer to tail");
    Profiler::StackPerfomance<DoublyLinkedList_StoredTail>("Doubly linked list with pointer to tail");
    Profiler::StackPerfomance<StackArray>("Dummy dynamic array");
    Profiler::StackPerfomance<StackArray_Capacity>("Dynamic array with reserved memory");
    return 0;
}