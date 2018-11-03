#include "LinkedList/LinkedList.h"
#include <cassert>
#include <random>
#include <vector>

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

template<typename T> using LinkedList_ = LinkedList<T, false, false>;
template<typename T> using DoublyLinkedList = LinkedList<T, true, false>;
template<typename T> using LinkedList_StoredTail = LinkedList<T, false, true>;
template<typename T> using DoublyLinkedList_StoredTail = LinkedList<T, true, true>;

int main() {
    StackTests<LinkedList_>();
    StackTests<LinkedList_StoredTail>();
    StackTests<DoublyLinkedList>();
    StackTests<DoublyLinkedList_StoredTail>();
    return 0;
}