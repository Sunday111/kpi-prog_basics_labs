#pragma once

namespace linked_list_impl
{
    template<typename Derived>
    class PrevNodeRef
    {
    public:
        Derived* previous = nullptr;
    };

    template<typename T, bool doublyLinked>
    class LinkedListNode :
        public std::conditional_t<doublyLinked, PrevNodeRef<LinkedListNode<T, doublyLinked>>, Empty>
    {
    public:
        template<typename... Args>
        LinkedListNode(Args&&... args) :
            data(T(std::forward<Args>(args)...))
        {
        }

        T data;
        std::unique_ptr<LinkedListNode> next;
    };

    template<typename T, bool doublyLinked>
    class TailNodeRef
    {
    public:
        LinkedListNode<T, doublyLinked>* tail = nullptr;
    };
    template<typename T, bool doublyLinked, bool is_const>
    class ListIterator
    {
    public:
        using NodeClean = LinkedListNode<T, doublyLinked>;
        using Node = std::conditional_t<is_const, std::add_const_t<NodeClean>, NodeClean>;
        using value_type = std::conditional_t<is_const, std::add_const_t<T>, T>;

    public:
        ListIterator(Node* node) :
            m_node(node)
        {
        }

        bool HasValue() const {
            return m_node != nullptr;
        }

        value_type& Value() const {
            return m_node->data;
        }

        void Advance() {
            m_node = m_node->next.get();
        }

    private:
        Node* m_node = nullptr;
    };
}


template<typename T, bool doublyLinked, bool storeTail>
LinkedList<T, doublyLinked, storeTail>::~LinkedList() {
    while (m_root) {
        PopBack();
    }
}

template<typename T, bool doublyLinked, bool storeTail>
typename LinkedList<T, doublyLinked, storeTail>::Iterator
LinkedList<T, doublyLinked, storeTail>::GetIterator() {
    return Iterator(m_root.get());
}

template<typename T, bool doublyLinked, bool storeTail>
typename LinkedList<T, doublyLinked, storeTail>::ConstIterator
LinkedList<T, doublyLinked, storeTail>::GetIterator() const {
    return ConstIterator(m_root.get());
}

template<typename T, bool doublyLinked, bool storeTail>
template<typename... Args>
void LinkedList<T, doublyLinked, storeTail>::EmplaceBack(Args&&... args) {
    auto makeNodePtr = [&] {
        return std::make_unique<Node>(std::forward<Args>(args)...);
    };

    if (m_root == nullptr) {
        m_root = makeNodePtr();

        if constexpr (storeTail) {
            this->tail = m_root.get();
        }

        return;
    }

    Node* tail = GetTail();

    tail->next = makeNodePtr();

    if constexpr (doublyLinked) {
        tail->next->previous = tail;
    }

    if constexpr (storeTail) {
        this->tail = tail->next.get();
    }
}

template<typename T, bool doublyLinked, bool storeTail>
std::optional<T> LinkedList<T, doublyLinked, storeTail>::PopBack() {
    std::optional<T> result;

    if (m_root != nullptr) {
        if constexpr (doublyLinked && storeTail) {
            this->tail = this->tail->previous;
            if (this->tail) {
                result = std::move(this->tail->next->data);
                this->tail->next = nullptr;
            }
            else {
                result = std::move(m_root->data);
                m_root = nullptr;
            }
        }
        else {
            // Only one element in the list
            if (!m_root->next) {
                result = std::move(m_root->data);
                m_root = nullptr;

                if constexpr (storeTail) {
                    this->tail = nullptr;
                }
            }
            else {
                Node* newTail = m_root.get();
                while (newTail->next->next != nullptr) {
                    newTail = newTail->next.get();
                }

                result = std::move(newTail->next->data);
                newTail->next = nullptr;

                if constexpr (storeTail) {
                    this->tail = newTail;
                }
            }
        }
    }

    return result;
}

template<typename T, bool doublyLinked, bool storeTail>
typename LinkedList<T, doublyLinked, storeTail>::Node*
LinkedList<T, doublyLinked, storeTail>::GetTail() {
    if constexpr (storeTail) {
        return this->tail;
    }
    else {
        Node* tail = m_root.get();
        while (tail->next != nullptr) {
            tail = tail->next.get();
        }
        return tail;
    }
}
