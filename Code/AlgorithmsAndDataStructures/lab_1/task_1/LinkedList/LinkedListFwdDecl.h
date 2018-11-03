#pragma once

namespace linked_list_impl
{
    template<typename T, bool doublyLinked, bool is_const>
    class ListIterator;

    template<typename T, bool doublyLinked>
    class TailNodeRef;

    template<typename T, bool doublyLinked>
    class LinkedListNode;

    class Empty {};

    template<typename T, bool doublyLinked, bool storeTail>
    using TailNodeStorage = std::conditional_t<storeTail, linked_list_impl::TailNodeRef<T, doublyLinked>, linked_list_impl::Empty>;
}
