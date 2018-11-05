#include "Array/Array.h"
#include "LinkedList/LinkedList.h"
#include <cassert>
#include <random>
#include <vector>
#include <chrono>
#include <iostream>

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

template<typename F, typename TimeScale = std::chrono::milliseconds>
auto GetProcessDuration(F&& f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<TimeScale>(end - start);
}

template
<
    template<typename> typename Layout,
    typename ValueGenerator,
    typename TimeScale = std::chrono::milliseconds
>
std::pair<long double, long double> StackProfile_PushPop(size_t repeats, size_t collection_size, ValueGenerator&& generator) {
    using value_type = decltype(generator());
    long double push_total = 0.0;
    long double pop_total = 0.0;

    for (size_t r = 0; r < repeats; ++r) {
        Layout<value_type> stack;

        auto push_duration = GetProcessDuration([&]() {
            for (size_t i = 0; i < collection_size; ++i) {
                stack.EmplaceBack(generator());
            }
        });

        push_total += static_cast<long double>(push_duration.count()) / repeats;

        auto pop_duration = GetProcessDuration([&]() {
            for (size_t i = 0; i < collection_size; ++i) {
                stack.PopBack();
            }
        });

        pop_total += static_cast<long double>(pop_duration.count()) / repeats;
    }

    return std::pair(push_total, pop_total);
}

struct NoCapacityPolicy
{
    std::size_t GetNewCapacity(std::size_t requestedCapacity, std::size_t) {
        return requestedCapacity;
    }
};

template<typename T> using LinkedList_ = LinkedList<T, false, false>;
template<typename T> using DoublyLinkedList = LinkedList<T, true, false>;
template<typename T> using LinkedList_StoredTail = LinkedList<T, false, true>;
template<typename T> using DoublyLinkedList_StoredTail = LinkedList<T, true, true>;
template<typename T> using StackArray = Array<T, NoCapacityPolicy>;
template<typename T> using StackArray_Capacity = Array<T, DefaultCapacityPolicy>;

int main() {
    StackTests<LinkedList_>();
    StackTests<LinkedList_StoredTail>();
    StackTests<DoublyLinkedList>();
    StackTests<DoublyLinkedList_StoredTail>();
    StackTests<StackArray>();
    StackTests<StackArray_Capacity>();

    //auto dummy_generator = []() {
    //    return 10;
    //};
    //
    //size_t constexpr pushesCount = 1000000;
    //
    //auto d1 = StackProfile_PushPop<LinkedList_>(1, pushesCount, dummy_generator);
    //std::cout << "Simple linked list:\n";
    //std::cout << "\tpush: " << d1.first << "ms \n";
    //std::cout << "\tpop: " << d1.second << "ms \n";
    //
    //auto d2 = StackProfile_PushPop<DoublyLinkedList>(1, pushesCount, dummy_generator);
    //std::cout << "Doubly linked list:\n";
    //std::cout << "\tpush: " << d2.first << "ms \n";
    //std::cout << "\tpop: " << d2.second << "ms \n";
    //
    //auto d3 = StackProfile_PushPop<LinkedList_StoredTail>(1, pushesCount, dummy_generator);
    //std::cout << "Simple linked list with cached tail:\n";
    //std::cout << "\tpush: " << d3.first << "ms \n";
    //std::cout << "\tpop: " << d3.second << "ms \n";
    //
    //auto d4 = StackProfile_PushPop<DoublyLinkedList_StoredTail>(1, pushesCount, dummy_generator);
    //std::cout << "Doubly linked list with cached tail:\n";
    //std::cout << "\tpush: " << d4.first << "ms \n";
    //std::cout << "\tpop: " << d4.second << "ms \n";
    //
    //auto d5 = StackProfile_PushPop<StackArray>(1, pushesCount, dummy_generator);
    //std::cout << "Stack Array:\n";
    //std::cout << "\tpush: " << d5.first << "ms \n";
    //std::cout << "\tpop: " << d5.second << "ms \n";
    //
    //auto d6 = StackProfile_PushPop<StackArray_Capacity>(1, pushesCount, dummy_generator);
    //std::cout << "Stack array with capacity:\n";
    //std::cout << "\tpush: " << d6.first << "ms \n";
    //std::cout << "\tpop: " << d6.second << "ms \n";

    return 0;
}