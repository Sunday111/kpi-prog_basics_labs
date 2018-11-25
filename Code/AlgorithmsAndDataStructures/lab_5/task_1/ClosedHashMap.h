#pragma once

#include <cassert>
#include <vector>

struct LinearProbingCollisionPolicy
{
    size_t operator()(size_t i, size_t hash) const {
        return hash + i;
    }
};

struct QuadraticProbingCollisionPolicy
{
    size_t operator()(size_t i, size_t hash) const {
        return hash + i * i;
    }
};

template
<
    typename Key,
    typename Value,
    typename Hasher,
    typename CollisionPolicy = LinearProbingCollisionPolicy
>
class ClosedHashMap
{
private:
    struct Hash
    {
        size_t value;
    };

    struct TableElement
    {
        Key key;
        Value value;
        bool has_value : 1;
    };
public:

    ClosedHashMap(size_t hashSize) :
        m_hashSize(hashSize)
    {
        UpdateCapacity();
    }

    template<typename... Args>
    Value* Emplace(Key key, Args&&... args) {
        if (m_size < m_table.size()) {
            const Hash hash = GetHash(key);
            for (size_t collisionIndex = 0; collisionIndex < m_table.size(); ++collisionIndex) {
                const size_t tableIndex = HashToTableIndex(collisionIndex, hash);
                TableElement& tableElement = m_table[tableIndex];
                if (!tableElement.has_value) {
                    tableElement.has_value = true;
                    tableElement.key = key;
                    new (&tableElement.value) Value(std::forward<Args>(args)...);
                    ++m_size;
                    return &tableElement.value;
                }

                assert(tableElement.key != key);
            }
        }

        return nullptr;
    }

    Value* Find(const Key& key) {
        TableElement* element = FindElement(key);
        return element ? &element->value : nullptr;
    }

    size_t GetCapacity() const
    {
        return m_table.size();
    }

private:
    Hash GetHash(const Key& key) {
        const Hash hash{ m_hasher(key, m_hashSize) };
        return hash;
    }

    TableElement* FindElement(const Key& key) {
        const Hash hash = GetHash(key);
        for (size_t collisionIndex = 0; collisionIndex < m_table.size(); ++collisionIndex) {
            const size_t tableIndex = HashToTableIndex(collisionIndex, hash);
            auto& tableElement = m_table[tableIndex];
            if (!tableElement.has_value) {
                break;
            }

            if (tableElement.key == key) {
                return &tableElement;
            }
        }

        return nullptr;
    }

    size_t HashToTableIndex(size_t collisionIndex, const Hash& hash) {
        const size_t index = m_collisionPolicy(collisionIndex, hash.value) % m_table.size();
        return index;
    }

    static size_t ComputeCapacity(size_t bytesCount) {
        size_t capacity = 1;
        for (size_t i = 0; i < bytesCount; ++i) {
            capacity *= 256;
        }

        return capacity;
    }

    void UpdateCapacity() {
        const size_t capacity = ComputeCapacity(m_hashSize);
        TableElement defaultElement;
        defaultElement.has_value = false;
        m_table.resize(capacity, defaultElement);
    }

private:
    Hasher m_hasher;
    CollisionPolicy m_collisionPolicy;
    size_t m_hashSize = 2;
    size_t m_size = 0;
    std::vector<TableElement> m_table;
};