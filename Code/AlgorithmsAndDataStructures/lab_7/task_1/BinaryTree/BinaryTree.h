#pragma once

#include <array>
#include <memory>
#include <type_traits>

enum class Direction
{
    Left,
    Right,
    Last__
};

Direction ReverseDirection(Direction dir) {
    if (dir == Direction::Left) {
        return Direction::Right;
    }

    return Direction::Left;
}

template<typename T>
class BinaryTreeLeaf;

template<typename T>
using BinaryTreeLeafPtr = std::unique_ptr<BinaryTreeLeaf<T>>;

template<typename T>
class BinaryTreeLeaf
{
public:
    static constexpr size_t LinksCount = static_cast<size_t>(Direction::Last__);

    template<typename... Args>
    BinaryTreeLeaf(Args&&... args);

    BinaryTreeLeafPtr<T>& GetLink(Direction direction);
    const BinaryTreeLeafPtr<T>& GetLink(Direction direction) const;

    T& GetLeafData();
    const T& GetLeafData() const;

private:
    T m_data;
    std::array<BinaryTreeLeafPtr<T>, LinksCount> m_links;
};

template<typename T>
template<typename... Args>
BinaryTreeLeaf<T>::BinaryTreeLeaf(Args&&... args) :
    m_data(std::forward<Args>(args)...)
{
}

template<typename T>
BinaryTreeLeafPtr<T>& BinaryTreeLeaf<T>::GetLink(Direction direction) {
    return m_links[static_cast<size_t>(direction)];
}

template<typename T>
const BinaryTreeLeafPtr<T>& BinaryTreeLeaf<T>::GetLink(Direction direction) const {
    return m_links[static_cast<size_t>(direction)];
}

template<typename T>
T& BinaryTreeLeaf<T>::GetLeafData() {
    return m_data;
}

template<typename T>
const T& BinaryTreeLeaf<T>::GetLeafData() const {
    return m_data;
}

template<typename T, typename... Args>
BinaryTreeLeafPtr<T> MakeLeaf(Args&&... args) {
    return std::make_unique<BinaryTreeLeaf<T>>(std::forward<Args>(args)...);
}


