#pragma once

#include <algorithm>

namespace sort
{
    template
    <
        typename T,
        typename Cmp = std::less<T>,
        typename Enable = std::enable_if_t<std::is_integral_v<T>>
    >
    void counting_sort(T* array, size_t size, T min, T max, Cmp cmp = Cmp{}) {
        if (size < 2) {
            return;
        }

        const T range = max - min + 1;
        std::vector<size_t> counts(range);

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
