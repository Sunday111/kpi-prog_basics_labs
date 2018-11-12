#pragma once

#include <algorithm>

namespace sort::merge_sort_impl
{
    template<typename T, typename Predicate>
    void merge_sort_merge(T* a, T* b, size_t left, size_t right, size_t end, Predicate&& predicate) {
        size_t i = left;
        size_t j = right;
        for (size_t k = left; k < end; ++k) {
            if (i < right && (j >= end || !!predicate(a[i], a[j]))) {
                b[k] = a[i];
                ++i;
            }
            else {
                b[k] = a[j];
                ++j;
            }
        }
    }
}

namespace sort
{
    template<typename T, typename Predicate = std::less<T>>
    void merge_sort(T* a, T* b, size_t n, Predicate&& predicate = Predicate{}) {
        using namespace merge_sort_impl;
        for (size_t width = 1; width < n; width *= 2) {
            for (size_t i = 0; i < n; i += 2 * width) {
                const size_t left = std::min(i + width, n);
                const size_t right = std::min(i + 2 * width, n);
                merge_sort_merge(a, b, i, left, right, predicate);
            }

            for (size_t i = 0; i < n; ++i) {
                a[i] = b[i];
            }
        }
    }
}
