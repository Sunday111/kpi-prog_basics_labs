#pragma once

#include <cassert>
#include "BinaryTree.h"
#include "PrintTree.h"

template<typename T>
class BinarySearchTree
{
private:
    using Leaf = BinaryTreeLeaf<T>;
    using LeafPtr = BinaryTreeLeafPtr<T>;

public:
    bool Emplace(T value);
    bool HasValue(const T& value);
    void Print(std::ostream& output);

private:
    static Direction SearchDirection(const BinaryTreeLeaf<T>& root, const T& value);

private:
    LeafPtr m_root;
};

template<typename T>
bool BinarySearchTree<T>::Emplace(T value) {
    if (m_root == nullptr) {
        m_root = MakeLeaf<T>(std::move(value));
        return true;
    }

    LeafPtr* pRoot = &m_root;
    while (*pRoot) {
        if ((*pRoot)->GetLeafData() == value) {
            return false;
        }

        const Direction direction = SearchDirection(**pRoot, value);
        pRoot = &(*pRoot)->GetLink(direction);
    }

    *pRoot = MakeLeaf<T>(std::move(value));
    return true;
}

template<typename T>
bool BinarySearchTree<T>::HasValue(const T& value) {
    Leaf* root = m_root.get();

    while (root) {
        if (value == root->GetLeafData()) {
            return true;
        }

        const auto dir = SearchDirection(*root, value);
        root = root->GetLink(dir).get();
    }

    return false;
}

template<typename T>
Direction BinarySearchTree<T>::SearchDirection(const BinaryTreeLeaf<T>& root, const T& value) {
    const T& nodeValue = root.GetLeafData();
    assert(nodeValue != value);
    return nodeValue < value ? Direction::Left : Direction::Right;
}

template<typename T>
void BinarySearchTree<T>::Print(std::ostream& output) {
    PrintTree<T>(m_root, output, [](const Leaf& leaf, std::ostream& output) {
        output << leaf.GetLeafData();
    });
}
