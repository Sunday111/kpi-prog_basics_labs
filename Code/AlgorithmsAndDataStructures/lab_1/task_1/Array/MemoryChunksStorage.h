#pragma once

#include "MemoryStorage.h"

template<size_t chunkSize, typename CapacityPolicy = DefaultCapacityPolicy>
class MemoryChunksStorage
{
public:
    std::byte* GetData() {
        return m_storage.GetData();
    }

    const std::byte* GetData() const {
        return m_storage.GetData();
    }

    std::size_t GetSize() const {
        return ToChunksCount(m_storage.GetSize());
    }

    void Resize(std::size_t chunksCount) {
        m_storage.Resize(ToBytesCount(chunksCount));
    }

    const void* At(std::size_t chunkIndex) const {
        return m_storage.At(ToBytesCount(chunkIndex));
    }

    void* At(std::size_t chunkIndex) {
        return m_storage.At(ToBytesCount(chunkIndex));
    }

protected:
    static constexpr std::size_t ToBytesCount(std::size_t chunksCount) {
        return chunksCount * chunkSize;
    }

    static constexpr std::size_t ToChunksCount(std::size_t bytesCount) {
        return bytesCount / chunkSize;
    }

private:
    MemoryStorage<CapacityPolicy> m_storage;
};