#pragma once

#include <algorithm>
#include <vector>

template
<
    typename Key,
    typename Value
>
class Bucket
{
protected:
    using KeyVal = std::pair<Key, Value>;
    using Collection = std::vector<KeyVal>;
    using Iterator = typename Collection::iterator;
    using ConstIterator = typename Collection::const_iterator;

public:
    template<typename... Args>
    Value* TryEmplace(Key key, Args&&... args) {
        auto it = LowerBound(key);
        if (IteratorPointsToKey(it, key)) {
            return nullptr;
        }
        it = m_values.insert(it, KeyVal(key, Value(std::forward<Args>(args)...)));
        return &it->second;
    }

    Value* Find(const Key& key) {
        const auto it = LowerBound(key);
        if (IteratorPointsToKey(it, key)) {
            return &it->second;
        }
        return nullptr;
    }

    const Value* Find(const Key& key) const {
        const auto it = LowerBound(key);
        if (IteratorPointsToKey(key)) {
            return &it->second;
        }
        return nullptr;
    }

private:
    Iterator LowerBound(const Key key) {
        return std::lower_bound(m_values.begin(), m_values.end(), key, [](auto& keyval, const Key& key) {
            return keyval.first < key;
        });
    }

    ConstIterator LowerBound(const Key key) const {
        return std::lower_bound(m_values.begin(), m_values.end(), key, [](auto& keyval, const Key& key) {
            return keyval.first < key;
        });
    }

    template<typename It>
    bool IteratorPointsToKey(const It& iterator, const Key& key) const {
        return
            iterator != m_values.end() &&
            iterator->first == key;
    }

private:
    Collection m_values;
};

template
<
    typename Key,
    typename Value,
    typename Hasher
>
class OpenHashMap
{
private:
    struct Hash
    {
        size_t value;
    };

    using Bucket = Bucket<Key, Value>;

public:
    OpenHashMap(size_t bucketsCount)
    {
        m_buckets.resize(bucketsCount);
    }

    template<typename... Args>
    Value* Emplace(Key key, Args&&... args) {
        Bucket& bucket = GetBucket(key);
        return bucket.TryEmplace(key, std::forward<Args>(args)...);
    }

    Value* Find(const Key& key) {
        Bucket& bucket = GetBucket(key);
        return bucket.Find(key);
    }

    const Value* Find(const Key& key) const {
        Bucket& bucket = GetBucket(key);
        return bucket.Find(key);
    }

private:
    Hash GetHash(const Key& key) const {
        const size_t value = m_hasher(key) % m_buckets.size();
        return Hash{ value };
    }

    Bucket& GetBucket(const Key& key) {
        const Hash hash = GetHash(key);
        return m_buckets[hash.value];
    }

    const Bucket& GetBucket(const Key& key) const {
        const Hash hash = GetHash(key);
        return m_buckets[hash.value];
    }

private:
    Hasher m_hasher;
    std::vector<Bucket> m_buckets;
};