#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <vector>

struct KnuthMultiplicativeMethod {
    size_t operator()(size_t key, uint8_t bytes) const {
        assert(bytes <= sizeof(key));
        const size_t bits = bytes * 8;
        return (key * 2654435761) % (size_t{ 1 } << bits);
    }
};

struct DummyHasher {
    size_t operator()(size_t key, uint8_t bytes) {
        assert(bytes <= sizeof(key));
        size_t result = 0;
        for (uint8_t i = 0; i < bytes; ++i) {
            const uint8_t byte_offset = (i % 2) ? (sizeof(key) / 2) : 0;
            const size_t bit_offset = byte_offset * 8;
            const size_t mask = 0xFF << bit_offset;
            result |= (mask & key);
        }
        return result;
    }
};

template<typename Value, typename Hasher>
class HashMap
{
private:
    struct KeyVal
    {
        size_t key;
        Value value;
        uint8_t has_value : 1;
    };
public:

    HashMap()
    {
        UpdateCapacity();
    }

    template<typename... Args>
    void Emplace(size_t key, Args&&... args) {
        if (m_size == m_table.size()) {
            throw std::exception("Need change hash size");
        }

        const size_t hash = m_hasher(key, m_hash_size);
        for (size_t i = 0; i < m_table.size(); ++i) {
            const size_t pos = (hash + i) & m_table.size();
            auto& keyVal = m_table[hash];
            if (!keyVal.has_value) {
                keyVal.has_value = 1ui8;
                keyVal.key = key;
                new (&keyVal.value) Value(std::forward<Args>(args)...);
                ++m_size;
                break;
            }
        }
    }

    Value* Find(size_t key) {
        const size_t hash = m_hasher(key, m_hash_size);
        for (size_t i = 0; i < m_table.size(); ++i) {
            const size_t pos = (hash + i) & m_table.size();
            auto& keyVal = m_table[hash];
            if (!keyVal.has_value) {
                break;
            }

            if (keyVal.key == key) {
                return &keyVal.value;
            }
        }
        return nullptr;
    }

private:
    void UpdateCapacity() {
        size_t capacity = 0;
        for (size_t i = 0; i < m_hash_size; ++i) {
            const size_t bit_offset = i * 8;
            capacity |= std::numeric_limits<uint8_t>::max() << bit_offset;
        }

        KeyVal d;
        d.has_value = 0ui8;
        m_table.resize(capacity, d);
    }

private:
    Hasher m_hasher;
    uint8_t m_hash_size = 2;
    size_t m_size = 0;
    std::vector<KeyVal> m_table;
};

int main(int, char**) {
    using T = double;
    using Duration = std::chrono::nanoseconds;
    using Clock = std::chrono::high_resolution_clock;

    constexpr size_t valuesCount = 60000;
    constexpr size_t stepsCount = 100;
    constexpr size_t valuesPerStep = valuesCount / stepsCount;
    constexpr size_t keyMax = std::numeric_limits<size_t>::max();
    constexpr size_t keyDelta = keyMax / valuesCount;

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
        auto dt = t2 - t1;
        return std::chrono::duration_cast<Duration>(dt).count();
    };

    std::vector<std::pair<size_t, T>> pairs;
    pairs.reserve(valuesCount);

    println("Adding new elements to hash table");
    println("Existing elements | Knuth hashing | Dummy hashing");

    HashMap<T, KnuthMultiplicativeMethod> map_a;
    HashMap<T, DummyHasher> map_b;

    for (size_t step = 1; step < stepsCount; ++step) {
        // generate keys
        for (size_t i = 0; i < valuesPerStep; ++i) {
            size_t key = keyDelta * (valuesPerStep * step + i);
            T value = static_cast<T>(key);
            pairs.push_back(std::pair{ key, value });
        }

        // profile insertions
        auto profileInsertions = [&](auto& map) {
            const size_t pairsBegin = valuesPerStep * (step - 1);
            const size_t pairsEnd = pairs.size() - 1;
            return getExecutionTime([&]() {
                for (size_t i = pairsBegin; i < pairsEnd; ++i) {
                    auto& pair = pairs[i];
                    map.Emplace(pair.first, pair.second);
                }
            });
        };

        auto duration_a = profileInsertions(map_a);
        auto duration_b = profileInsertions(map_b);

        println(valuesPerStep * (step - 1), ',',  duration_a, ',', duration_b);
    }
    println();

    return 0;
}