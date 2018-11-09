#pragma once

#include "MemoryChunksStorage.h"

template<typename T, bool is_const>
class ArrayIterator
{
public:
    using value_type = std::conditional_t<is_const, std::add_const_t<T>, T>;

    bool HasValue() const {
        return index < size;
    }

    value_type& Value() const {
        return pointer[index];
    }

    void Advance() {
        ++index;
    }

    size_t index = 0;
    size_t size = 0;
    value_type* pointer = nullptr;
};

template<typename T, typename CapacityPolicy = DefaultArrayPolicy>
class Array
{
public:
    using Iterator = ArrayIterator<T, false>;
    using ConstIterator = ArrayIterator<T, true>;

public:
    template<typename... Args>
    void EmplaceBack(Args... args) {
        const size_t oldSize = m_chunks.GetSize();
        m_chunks.Resize(oldSize + 1);
        void* pObject = m_chunks.At(oldSize);
        new (pObject) T(std::forward<Args>(args)...);
    }

    void PopBack() {
        const std::size_t size = m_chunks.GetSize();
        if (size > 0) {
            m_chunks.Resize(size - 1);
        }
    }

    T* At(std::size_t index) {
        return reinterpret_cast<T*>(m_chunks.At(index));
    }

    const T* At(std::size_t index) const {
        return reinterpret_cast<const T*>(m_chunks.At(index));
    }

    Iterator GetIterator() {
        return MakeIterator<Iterator>();
    }

    ConstIterator GetIterator() const {
        return MakeIterator<ConstIterator>();
    }

    template<typename TIterator>
    TIterator MakeIterator() {
        TIterator i;
        i.index = 0;
        i.size = m_chunks.GetSize();
        if (m_chunks.GetSize() > 0) {
            i.pointer = At(0);
        }
        return i;
    }

private:
    MemoryChunksStorage<sizeof(T), CapacityPolicy> m_chunks;
};