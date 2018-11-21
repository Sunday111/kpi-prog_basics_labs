#pragma once

#include <algorithm>
#include <type_traits>

namespace sort::radix_sort_impl
{
    template<typename type>
    constexpr type mask_by_bit_index(const size_t bit_index) {
        return (type{ 1 } << bit_index);
    }

    template<typename type>
    constexpr bool bit_is_set(const type value, const size_t bit_index) {
        return (mask_by_bit_index<type>(bit_index) & value) != 0;
    }

    template<bool ascending, typename element_type>
    size_t bit_partition(element_type* array, size_t size, size_t bit_index) {
        // Additional abstraction to support descending order
        auto first_part_object = [&](const element_type value) {
            if constexpr (ascending) {
                return !bit_is_set(value, bit_index);
            }
            else {
                return bit_is_set(value, bit_index);
            }
        };

        // Find index of value that must be in the second part
        size_t second_part_begin = 0;
        for (size_t i = 0; i < size; ++i) {
            if (!first_part_object(array[i])) {
                second_part_begin = i;
                break;
            }
        }

        // Actual partitioning
        for (size_t i = second_part_begin + 1; i < size; ++i) {
            if (first_part_object(array[i])) {
                std::swap(array[i], array[second_part_begin]);
                ++second_part_begin;
            }
        }

        // Update 'second_part_begin'
        // (case when all values in the same part)
        if (first_part_object(array[second_part_begin])) {
            ++second_part_begin;
        }

        return second_part_begin;
    }

    template<bool ascending, typename element_type>
    void radix_sort_msd_at(element_type* array, size_t size, size_t bit_index) {
        if (size == 0) {
            return;
        }

        // Do binary array partition by bit at bit_index
        size_t second_part_begin = bit_partition<ascending>(array, size, bit_index);

        // Sort parts recursively
        if (bit_index > 0) {
            // Sort parts recursively by bit at next index
            const size_t next_bit_index = bit_index - 1;

            // Sort first part
            radix_sort_msd_at<ascending>(array, second_part_begin, next_bit_index);

            // Sort second part
            radix_sort_msd_at<ascending>(array + second_part_begin, size - second_part_begin, next_bit_index);
        }
    }

    template<typename element_type>
    element_type max_array_element(const element_type* array, size_t size) {
        element_type max_element = std::numeric_limits<element_type>::lowest();
        for (size_t i = 0; i < size; ++i) {
            max_element = std::max(max_element, array[i]);
        }
        return max_element;
    }

    template<typename type>
    constexpr size_t most_significant_bit_index(const type value) {
        constexpr size_t bytes_count = sizeof(type);
        constexpr size_t bits_count = bytes_count * 8;
        size_t msbi = 0;
        for (size_t i = 1; (i < bits_count) && (mask_by_bit_index<type>(i) <= value); ++i) {
            if (bit_is_set(value, i)) {
                msbi = i;
            }
        }
        return msbi;
    }
}

namespace sort
{
    template
    <
        typename element_type,
        bool ascending = true,
        typename enable = std::enable_if_t<std::is_integral_v<element_type>>
    >
    void radix_sort_msd(element_type* array, size_t size) {
        using namespace radix_sort_impl;
        const element_type max_element = max_array_element(array, size);
        const size_t msbi = most_significant_bit_index(max_element);
        radix_sort_msd_at<ascending>(array, size, msbi);
    }
    
    template
    <
        typename element_type,
        bool ascending = true,
        typename enable = std::enable_if_t<std::is_integral_v<element_type>>
    >
    void radix_sort_lsd(element_type* array, element_type* buffer, size_t size) {
        using namespace radix_sort_impl;

        if (size < 2) {
            return;
        }

        const element_type max_element = max_array_element(array, size);
        const size_t msbi = most_significant_bit_index(max_element);
        for (size_t bit_index = 0; bit_index <= msbi; ++bit_index) {
            // Additional abstraction to support descending order
            auto first_part_object = [&](const element_type value) {
                if constexpr (ascending) {
                    return !bit_is_set(value, bit_index);
                }
                else {
                    return bit_is_set(value, bit_index);
                }
            };

            // 1. Copy first part elements to the beginning of buffer
            // and the second part objects to the end
            size_t next_fp = 0;
            size_t next_sp = size - 1;
            for (size_t i = 0; i < size; ++i) {
                const element_type value = array[i];
                if (first_part_object(value)) {
                    buffer[next_fp++] = value;
                }
                else {
                    buffer[next_sp--] = value;
                }
            }

            // 2. Copy elements back to array
            // 2.1. Copy first part
            for (size_t i = 0; i < next_fp; ++i) {
                array[i] = buffer[i];
            }

            // 2.2. Copy second part in reversed order
            const size_t last_idx = size - 1;
            for (size_t i = last_idx; i > next_sp; --i) {
                array[next_fp + (last_idx - i)] = buffer[i];
            }
        }
    }
}
