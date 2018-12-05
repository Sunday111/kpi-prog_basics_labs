#include <array>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <vector>

#include "ClosedHashMap.h"

template
<
    typename T,
    typename Enable = std::enable_if_t<std::is_integral_v<T>>
>
struct KnuthMultiplicativeMethod {
    size_t operator()(T key, size_t bytes) const {
        assert(bytes <= sizeof(key));
        const size_t bits = bytes * 8;
        return (key * T{ 2654435761 }) % (T{ 1 } << bits);
    }

    size_t operator()(T key) const {
        constexpr size_t bytes = 2;
        constexpr size_t bits = bytes * 8;
        assert(bytes <= sizeof(key));
        return (key * T{ 2654435761 }) % (T{ 1 } << bits);
    }
};

template
<
    typename T,
    typename Enable = std::enable_if_t<std::is_integral_v<T>>
>
struct FirstNBitsHasher {
    size_t operator()(T key, size_t bytes) const {
        assert(bytes <= sizeof(key));
        T result = 0;
        std::memcpy(&result, &key, bytes);
        return result;
    }

    size_t operator()(T key) const {
        assert(bytes <= sizeof(key));
        T result = 0;
        constexpr size_t bytes = 2;
        std::memcpy(&result, &key, bytes);
        return result;
    }
};

#ifdef _DEBUG
constexpr bool validateHashMap = true;
#else
constexpr bool validateHashMap = false;
#endif

using Key = size_t;
using Value = double;
using Duration = std::chrono::nanoseconds;
using DurationU = long double;
using Clock = std::chrono::high_resolution_clock;

template<typename Probing>
using HashMap = ClosedHashMap<Key, Value, KnuthMultiplicativeMethod<Key>, Probing>;

void Main() {
    const size_t valuesCount = std::numeric_limits<uint16_t>::max() + 1;
    const size_t stepsCount = 256;
    const size_t valuesPerStep = valuesCount / stepsCount;
    constexpr size_t keyAbsoluteValue = std::numeric_limits<size_t>::max();
    constexpr auto maxBucketsCount = size_t(std::numeric_limits<uint16_t>::max()) + 1;
    constexpr const char split = ';';
    constexpr size_t hasBytesCount = 2;

    constexpr size_t keyMinValues[]{
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        keyAbsoluteValue / 10,
        keyAbsoluteValue / 100,
        keyAbsoluteValue / 1000,
        keyAbsoluteValue / 10000,
        keyAbsoluteValue / 100000,
        keyAbsoluteValue / 1000000,
        keyAbsoluteValue / 10000000,
        keyAbsoluteValue / 100000000,
        keyAbsoluteValue / 1000000000,
        keyAbsoluteValue / 10000000000,
    };

    constexpr size_t keyMaxValues[]{
        keyAbsoluteValue,
        keyAbsoluteValue / 10,
        keyAbsoluteValue / 100,
        keyAbsoluteValue / 1000,
        keyAbsoluteValue / 10000,
        keyAbsoluteValue / 100000,
        keyAbsoluteValue / 1000000,
        keyAbsoluteValue / 10000000,
        keyAbsoluteValue / 100000000,
        keyAbsoluteValue / 1000000000,
        keyAbsoluteValue / 10000000000,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
        keyAbsoluteValue,
    };

    static_assert(sizeof(keyMaxValues) == sizeof(keyMinValues));
    const size_t passesCount = sizeof(keyMaxValues) / sizeof(keyMaxValues[0]);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::ostream& output = std::cout;

    auto print = [&output](auto... values) {
        (output << ... << values);
    };

    auto println = [&print](auto... values) {
        print(values..., '\n');
    };

    auto getExecutionTime = [](auto&& fn) {
        auto t1 = Clock::now();
        fn();
        auto t2 = Clock::now();
        return static_cast<DurationU>(std::chrono::duration_cast<Duration>(t2 - t1).count());
    };

    std::vector<std::vector<std::pair<DurationU, DurationU>>> addDurations(passesCount);
    std::vector<std::vector<std::pair<DurationU, DurationU>>> findDurations(passesCount);
    for (size_t pass = 0; pass < passesCount; ++pass) {
        std::vector<std::pair<Key, Value>> pairs;
        std::uniform_int_distribution<Key> keyDistribution(keyMinValues[pass], keyMaxValues[pass]);
        std::uniform_real_distribution<Value> valueDistribution(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        pairs.reserve(valuesCount);
        {
            // generate keys for the whole pass
            for (size_t i = 0; i < valuesCount; ++i) {
                // Make unique and random key
                auto it = pairs.begin();
                Key key;
                do
                {
                    key = keyDistribution(gen);
                    it = std::lower_bound(pairs.begin(), pairs.end(), key, [](auto& p1, Key key) {
                        return p1.first < key;
                    });
                } while (it != pairs.end() && it->first == key);

                auto value = valueDistribution(gen);
                pairs.insert(it, std::pair{ key, value });
            }

            // shuffle pairs
            std::shuffle(pairs.begin(), pairs.end(), gen);
        }

        HashMap<LinearProbingCollisionPolicy> map_a(hasBytesCount);
        HashMap<QuadraticProbingCollisionPolicy> map_b(hasBytesCount);
        auto& passEmplaceDurations = addDurations[pass];
        auto& passFindDurations = findDurations[pass];
        for (size_t step = 0; step < stepsCount - 1; ++step) {
            const size_t pairsBegin = valuesPerStep * step;
            const size_t minCapacity = std::min(map_a.GetCapacity(), map_b.GetCapacity());
            const size_t pairsEnd = std::min(pairsBegin + valuesPerStep, minCapacity);
            const size_t valuesOnStep = pairsEnd - pairsBegin;

            auto validate_no_value = [](auto& map, size_t key) {
                if constexpr (validateHashMap) {
                    auto pValue = map.Find(key);
                    if (pValue) {
                        throw std::runtime_error("Element in collection bug it must not be there");
                    }
                }
            };

            auto validate_has_value = [](auto& map, Key key, Value value) {
                if constexpr (validateHashMap) {
                    auto pValue = map.Find(key);
                    if (!pValue || *pValue != value) {
                        throw std::runtime_error("Element not in collection bug it must be there");
                    }
                }
            };

            auto emplace = [&](auto& map, Key key, Value value) {
                validate_no_value(map, key);
                auto result = map.Emplace(key, value);
                if constexpr (validateHashMap) {
                    if (result == nullptr) {
                        throw std::runtime_error("Failed to emplace object");
                    }
                }
                validate_has_value(map, key, value);
            };

            // profile emplace
            {
                auto profileAdd = [&](auto& map) {
                    return getExecutionTime([&]() {
                        for (size_t i = pairsBegin; i < pairsEnd; ++i) {
                            auto&[key, value] = pairs[i];
                            emplace(map, key, value);
                        }
                    });
                };

                auto duration_a = profileAdd(map_a) / valuesOnStep;
                auto duration_b = profileAdd(map_b) / valuesOnStep;
                passEmplaceDurations.push_back(std::pair{ duration_a, duration_b });
            }

            {
                // profile find
                auto profileFind = [&](auto& map) {
                    return getExecutionTime([&]() {
                        for (size_t i = pairsBegin; i < pairsEnd; ++i) {
                            auto&[key, value] = pairs[i];
                            const volatile auto pValue = map.Find(key);
                            validate_has_value(map, key, value);
                        }
                    });
                };

                auto duration_a = profileFind(map_a) / valuesOnStep;
                auto duration_b = profileFind(map_b) / valuesOnStep;
                passFindDurations.push_back(std::pair{ duration_a, duration_b });
            }
        }
    }

    auto showResults = [&](std::vector<std::vector<std::pair<DurationU, DurationU>>>& durations) {
        for (size_t step = 0; step < durations.front().size(); ++step) {
            auto getMean = [&](DurationU std::pair<DurationU, DurationU>::*pMember) {
                long double sum = 0;
                for (size_t pass = 0; pass < passesCount; ++pass) {
                    const auto& durationsPair = durations[pass][step];
                    const DurationU duration = (durationsPair.*pMember);
                    sum += duration;
                }
                return (sum / passesCount);
            };

            auto mean_a = getMean(&std::pair<DurationU, DurationU>::first);
            auto mean_b = getMean(&std::pair<DurationU, DurationU>::second);
            float occupiedElementsPercent = static_cast<float>(valuesPerStep * step * 100) / valuesCount;
            println(occupiedElementsPercent, split, mean_a, split, mean_b);
        }
    };

    {
        println("Adding new elements to hash table");
        println("Occupied elements percent", split, "Linear probing", split, "Quadratic probing");
        showResults(addDurations);
        println();
    }

    {
        println("Find existing elements");
        println("Occupied elements percent", split, "Linear probing", split, "Quadratic probing");
        showResults(findDurations);
        println();
    }
}

int main(int, char**) {
    try
    {
        Main();
    }
    catch (const std::exception& ex) {
        std::cout << "Exception :" << ex.what() << std::endl;
        return 1;
    }
    return 0;
}