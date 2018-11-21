#pragma once

#include <algorithm>

namespace sort::quick_sort_impl
{
    template<typename T, typename Predicate>
    T select_pivot(T* arr, int low, int high, Predicate& predicate) {
        const int mid = (low + high) / 2;

        if (predicate(arr[mid], arr[low])) {
            std::swap(arr[mid], arr[low]);
        }

        if (predicate(arr[high], arr[low])) {
            std::swap(arr[high], arr[low]);
        }

        if (predicate(arr[mid], arr[high])) {
            std::swap(arr[mid], arr[high]);
        }

        return arr[high];
    }

    template<typename T, typename Predicate>
    int hoare_partition(T* arr, int low, int high, Predicate& predicate) {
        const T pivot = select_pivot(arr, low, high, predicate);
        int i = low - 1;
        int j = high + 1;
        while (true) {
            while (predicate(arr[++i], pivot));
            while (predicate(pivot, arr[--j]));
            if (i >= j) {
                return j;
            }

            std::swap(arr[i], arr[j]);
        }
    }

    template<typename T, typename Predicate>
    void quick_sort_impl(T* array, int low, int high, Predicate& predicate) {
        if (low < high) {
            int p = hoare_partition(array, low, high, predicate);
            quick_sort_impl(array, low, p, predicate);
            quick_sort_impl(array, p + 1, high, predicate);
        }
    }
}

namespace sort
{
    template<typename T, typename Predicate = std::less<T>>
    void quick_sort(T* array, size_t size, Predicate predicate = Predicate()) {
        quick_sort_impl::quick_sort_impl(array, 0, static_cast<int>(size) - 1, predicate);
    }
}
