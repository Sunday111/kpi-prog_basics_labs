#pragma once

#include <algorithm>

namespace sort
{
    /// Uses counting sort algorithm to sort array
    /// Will work for integral numbers only
    /// Effective when collection size is much greater than numbers range
    /// Also accepts 'counts' additional array of size max - min + 1
    template
    <
        typename T,
        typename Enable = std::enable_if_t<std::is_integral_v<T>>
    >
    void counting_sort(T* array, T* counts, size_t size, T min, T max) {
        if (size < 2) {
            return;
        }

        const size_t range = max - min + 1;
        std::fill_n(counts, range, 0);

        for (T i = 0; i < size; ++i) {
            ++counts[array[i] - min];
        }

        for (T i = min; i <= max; ++i) {
            const size_t count = counts[i - min];
            for (size_t j = 0; j < count; ++j) {
                *array++ = i;
            }
        }
    }
}
