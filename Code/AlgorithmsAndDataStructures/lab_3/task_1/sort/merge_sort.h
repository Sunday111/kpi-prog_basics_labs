#pragma once

#include <algorithm>

namespace sort::merge_sort_impl
{
    template<typename T, typename Predicate = std::less<T>>
    void merge_sort_merge(T* a, T* b, size_t left, size_t right, size_t end) {
        size_t i = left;
        size_t j = right;
        for (size_t k = left; k < end; ++k) {
            if (i < right && (j >= end || a[i] <= a[j])) {
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
    void merge_sort(T* a, T* b, size_t n) {
        using namespace merge_sort_impl;
        for (size_t width = 1; width < n; width *= 2) {
            for (size_t i = 0; i < n; i += 2 * width) {
                merge_sort_merge(a, b, i, std::min(i + width, n), std::min(i + 2 * width, n));
            }

            for (size_t i = 0; i < n; ++i) {
                a[i] = b[i];
            }
        }
    }
}
