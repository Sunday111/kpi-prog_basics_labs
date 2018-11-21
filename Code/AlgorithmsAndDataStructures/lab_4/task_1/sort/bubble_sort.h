#pragma once

#include <algorithm>

namespace sort
{
    template<typename T, typename Cmp = std::less<T>>
    void bubble_sort(T* arr, size_t size, Cmp cmp = Cmp{}) {
        size_t n = size;
        size_t nn;
        do
        {
            nn = 0;
            for (size_t i = 1; i < n; ++i) {
                if (cmp(arr[i], arr[i - 1])) {
                    std::swap(arr[i - 1], arr[i]);
                    nn = i;
                }
            }
            n = nn;
        } while (n > 1);
    }
}
