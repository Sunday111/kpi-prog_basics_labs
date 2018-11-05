#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>

struct DefaultCapacityPolicy
{
    std::size_t GetNewCapacity(std::size_t requestedCapacity, std::size_t currentCapacity) {
        return std::max(requestedCapacity, currentCapacity + currentCapacity / 2);
    }
};

template<typename CapacityPolicy = DefaultCapacityPolicy>
class MemoryStorage : public CapacityPolicy
{
public:
    void Reserve(size_t requestedCapacity) {
        if (requestedCapacity <= m_capacity) {
            return;
        }

        const std::size_t newCapacity = this->GetNewCapacity(requestedCapacity, m_capacity);

        std::unique_ptr<std::byte[]> newData;
        newData.reset(new std::byte[newCapacity]);
        if (m_size > 0) {
            std::memcpy(newData.get(), m_data.get(), m_size);
        }
        std::swap(m_data, newData);
        m_capacity = newCapacity;
    }

    void Resize(std::size_t newSize) {
        Reserve(newSize);
        m_size = newSize;
    }

    const void* At(std::size_t index) const {
        assert(index < m_size);
        return &m_data[index];
    }

    void* At(std::size_t index) {
        assert(index < m_size);
        return &m_data[index];
    }

    std::size_t GetSize() const { return m_size; }

private:
    std::size_t m_size = 0;
    std::size_t m_capacity = 0;
    std::unique_ptr<std::byte[]> m_data;
};