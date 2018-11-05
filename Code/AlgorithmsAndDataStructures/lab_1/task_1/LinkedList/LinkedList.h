#pragma once

#include <memory>
#include <type_traits>
#include <optional>
#include "LinkedListFwdDecl.h"

template<typename T, bool doublyLinked, bool storeTail>
class LinkedList : protected linked_list_impl::TailNodeStorage<T, doublyLinked, storeTail>
{
public:
    using Iterator = linked_list_impl::ListIterator<T, doublyLinked, false>;
    using ConstIterator = linked_list_impl::ListIterator<T, doublyLinked, true>;

public:
    ~LinkedList();

    Iterator GetIterator();
    ConstIterator GetIterator() const;

    template<typename... Args>
    void EmplaceBack(Args&&... args);
    std::optional<T> PopBack();

protected:
    using Node = linked_list_impl::LinkedListNode<T, doublyLinked>;

protected:
    Node* GetTail();

private:
    std::unique_ptr<Node> m_root;
};

#include "LinkedListImpl.h"
