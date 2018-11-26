#pragma once

#include "BinaryTree.h"
#include "PrintTree.h"

template<typename T>
class RedBlackTreeNode
{
public:
    template<typename... Args>
    RedBlackTreeNode(Args&&... args);

    T& GetNodeData();
    const T& GetNodeData() const;

    bool IsRed() const { return m_red; }
    void MakeRed() { m_red = true; }
    void MakeBlack() { m_red = false; }

private:
    T m_data;
    bool m_red;
};

template<typename T>
class RedBlackTree
{
private:
    using Node = RedBlackTreeNode<T>;
    using Leaf = BinaryTreeLeaf<Node>;
    using LeafPtr = BinaryTreeLeafPtr<Node>;

public:
    bool Emplace(T value);
    bool HasValue(const T& value);
    void Print(std::ostream& output);

private:
    static Direction SearchDirection(const Leaf& root, const T& value);
    static Leaf* Insert(Leaf* root, const T& value);
    static Leaf* SingleRotation(Leaf* root, Direction dir);
    static Leaf* DoubleRotation(Leaf* root, Direction dir);

private:
    LeafPtr m_root;
};

template<typename T>
template<typename... Args>
RedBlackTreeNode<T>::RedBlackTreeNode(Args&&... args) :
    m_data(std::forward<Args>(args)...),
    m_red(true)
{}

template<typename T>
T& RedBlackTreeNode<T>::GetNodeData() {
    return m_data;
}

template<typename T>
const T& RedBlackTreeNode<T>::GetNodeData() const {
    return m_data;
}

template<typename T>
bool RedBlackTree<T>::Emplace(T value) {
    Leaf* const newRoot = Insert(m_root.get(), value);
    if (newRoot == nullptr) {
        return false;
    }
    m_root.release();
    m_root.reset(newRoot);
    newRoot->GetLeafData().MakeBlack();
    return true;
}

template<typename T>
bool RedBlackTree<T>::HasValue(const T& value) {
    Leaf* root = m_root.get();

    while (root) {
        if (value == root->GetLeafData().GetNodeData()) {
            return true;
        }

        const auto dir = SearchDirection(*root, value);
        root = root->GetLink(dir).get();
    }

    return false;
}

template<typename T>
Direction RedBlackTree<T>::SearchDirection(const Leaf& root, const T& value) {
    const T& nodeValue = root.GetLeafData().GetNodeData();
    assert(nodeValue != value);
    return nodeValue < value ? Direction::Left : Direction::Right;
}

template<typename T>
BinaryTreeLeaf<RedBlackTreeNode<T>>* RedBlackTree<T>::Insert(Leaf* root, const T& value) {
    if (root == nullptr) {
        return new Leaf(value);
    }

    if (value == root->GetLeafData().GetNodeData()) {
        return nullptr;
    }

    const auto dir = SearchDirection(*root, value);

    // recursive insert
    {
        auto& dirNode = root->GetLink(dir);
        auto newRoot = Insert(root->GetLink(dir).get(), value);
        if (newRoot == nullptr) {
            return nullptr;
        }
        dirNode.release();
        dirNode.reset(newRoot);
    }

    // rebalance
    auto isRed = [](LeafPtr& leaf) {
        return leaf != nullptr && leaf->GetLeafData().IsRed();
    };

    if (isRed(root->GetLink(dir))) {
        const Direction revDir = ReverseDirection(dir);
        if (isRed(root->GetLink(revDir))) {
            root->GetLeafData().MakeRed();
            root->GetLink(Direction::Left)->GetLeafData().MakeBlack();
            root->GetLink(Direction::Right)->GetLeafData().MakeBlack();
        }
        else {
            if (isRed(root->GetLink(dir)->GetLink(dir))) {
                root = SingleRotation(root, revDir);
            }
            else if (isRed(root->GetLink(dir)->GetLink(revDir))) {
                root = DoubleRotation(root, revDir);
            }
        }
    }

    return root;
}

template<typename T>
BinaryTreeLeaf<RedBlackTreeNode<T>>* RedBlackTree<T>::SingleRotation(Leaf* root, Direction dir) {
    const Direction revDir = ReverseDirection(dir);
    const auto save = root->GetLink(revDir).release();

    {
        const auto tmp = save->GetLink(dir).release();
        root->GetLink(revDir).reset(tmp);
    }

    save->GetLink(dir).reset(root);

    root->GetLeafData().MakeRed();
    save->GetLeafData().MakeBlack();

    return save;
}

template<typename T>
BinaryTreeLeaf<RedBlackTreeNode<T>>* RedBlackTree<T>::DoubleRotation(Leaf* root, Direction dir) {
    {
        const Direction revDir = ReverseDirection(dir);
        auto tmp = SingleRotation(root->GetLink(revDir).get(), revDir);
        auto& pLeaf = root->GetLink(revDir);
        pLeaf.release();
        pLeaf.reset(tmp);
    }

    return SingleRotation(root, dir);
}


template<typename T>
void RedBlackTree<T>::Print(std::ostream& output) {
    PrintTree<Node>(m_root, output, [](const Leaf& leaf, std::ostream& output) {
        output << leaf.GetLeafData().GetNodeData();
    });
}