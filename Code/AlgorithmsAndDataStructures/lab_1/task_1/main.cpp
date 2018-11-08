#include "Array/Array.h"
#include "LinkedList/LinkedList.h"
#include "Thread/ThreadPool.h"
#include <cassert>
#include <random>
#include <vector>
#include <chrono>
#include <iostream>
#include <string>
#include "Logger.h"

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
    typename Logger,
    typename TimeScale = std::chrono::milliseconds,
    typename DurationType = long double
>
class OperationProfiler
{ 
public:
    using UserOperation = std::function<void(OnePassState&)>;

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

    std::vector<OperationInfo> DoProfiling(ThreadPool<Logger>& threadPool) {
        std::vector<std::vector<TimeScale>> passesInfo;
        passesInfo.resize(m_passesCount);

        for (size_t i = 0; i < m_passesCount; ++i) {
            threadPool.AddTask([&, i]() {
                OnePassState state;
                std::vector<TimeScale> passInfo;
                for (Operation& operation : m_algorithm) {
                    TimeScale duration = GetProcessDuration<TimeScale>([&]() {
                        operation.function(state);
                    });
                    passInfo.push_back(duration);
                }

                passesInfo[i] = std::move(passInfo);
            });
        }
        threadPool.Wait();

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

        for (std::vector<TimeScale>& passInfo : passesInfo) {
            for (size_t i = 0; i < passInfo.size(); ++i) {
                OperationInfo& operationInfo = operationsInfo[i];
                auto duration = static_cast<DurationType>(passInfo[i].count());
                operationInfo.min = std::min(operationInfo.min, duration);
                operationInfo.max = std::max(operationInfo.max, duration);
                operationInfo.mean += duration / m_passesCount;
            }
        }

        return operationsInfo;
    }

private:
    size_t m_passesCount = 1;
    std::vector<Operation> m_algorithm;
};

struct NoCapacityPolicy
{
    std::size_t GetNewCapacity(std::size_t requestedCapacity, std::size_t) {
        return requestedCapacity;
    }
};

template<typename Element, typename Logger>
class StackProfiler
{
public:
    StackProfiler(std::ostream& output) :
        m_logger(output),
        m_threadPool(&m_logger)
    {
    }

    template
    <
        template<typename> typename Layout
    >
    void StackPerfomance(std::string title) {
        using StateType = Layout<Element>;
        OperationProfiler<StateType, Logger> profiler;
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

        StartPool();

        auto operationsInfo = profiler.DoProfiling(m_threadPool);
        m_logger.Write(title);
        for (auto& operationInfo : operationsInfo) {
            m_logger.Write('\t', operationInfo.name);
            m_logger.Write("\t\tmin:  ", operationInfo.min, "ms");
            m_logger.Write("\t\tmean:  ", operationInfo.mean, "ms");
            m_logger.Write("\t\tmax:  ", operationInfo.max, "ms");
        }
        m_logger.Write();
    }

    ~StackProfiler() {
        m_threadPool.StopAndWait();
    }

    size_t operations = 1;
    size_t passes = 1;
    size_t threads = 0;

protected:
    void StartPool() {
        if (!m_poolStarted) {
            m_threadPool.SetDesiredThreadsCount(threads);
            m_threadPool.Start();
            m_poolStarted = true;
        }
    }

private:
    bool m_poolStarted = false;
    Logger m_logger;
    ThreadPool<Logger> m_threadPool;
};

template<typename T> using LinkedList_ = LinkedList<T, false, false>;
template<typename T> using DoublyLinkedList = LinkedList<T, true, false>;
template<typename T> using LinkedList_StoredTail = LinkedList<T, false, true>;
template<typename T> using DoublyLinkedList_StoredTail = LinkedList<T, true, true>;
template<typename T> using StackArray = Array<T, NoCapacityPolicy>;
template<typename T> using StackArray_Capacity = Array<T, DefaultCapacityPolicy>;

int main() {
    using Profiler = StackProfiler<int, Log::Logger<>>;
    Profiler profiler(std::cout);
    profiler.operations = 10;
    profiler.passes = 100;
    profiler.threads = 0;
    
    profiler.StackPerfomance<LinkedList_>("Linked list");
    profiler.StackPerfomance<DoublyLinkedList>("Doubly linked list");
    profiler.StackPerfomance<LinkedList_StoredTail>("Linked list with pointer to tail");
    profiler.StackPerfomance<DoublyLinkedList_StoredTail>("Doubly linked list with pointer to tail");
    profiler.StackPerfomance<StackArray>("Dummy dynamic array");
    profiler.StackPerfomance<StackArray_Capacity>("Dynamic array with reserved memory");
    //using Logger = Log::Logger<>;
    //using ThreadPool = ThreadPool<Logger>;
    //
    //constexpr size_t tasks_count = 40;
    //
    //Logger logger(std::cout);
    //ThreadPool threadPool(&logger);
    //threadPool.Start();
    //for (size_t i = 0; i < tasks_count; ++i) {
    //    logger.Write("Task ", i, " begins");
    //    threadPool.AddTask([&, i] {
    //        logger.Write("Task ", i, " begins");
    //        const int s = 2000 + rand() % 2000;
    //        std::this_thread::sleep_for(std::chrono::milliseconds(s));
    //        logger.Write("Task ", i, " ends");
    //    });
    //    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    //}
    //threadPool.StopAndWait();
    //system("pause");
    //
    //return 0;
}