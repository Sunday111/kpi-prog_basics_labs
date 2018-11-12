#pragma once

#include <algorithm>

namespace sort::heap_sort_impl
{
    template<typename T, typename Predicate>
    void heapify(T* arr, int n, int i, Predicate&& pred) {
        int largest = i; // Initialize largest as root 
        int l = 2 * i + 1; // left = 2*i + 1 
        int r = 2 * i + 2; // right = 2*i + 2 

        // If left child is larger than root 
        if (l < n && !pred(arr[l], arr[largest]))
            largest = l;

        // If right child is larger than largest so far 
        if (r < n && !pred(arr[r], arr[largest]))
            largest = r;

        // If largest is not root 
        if (largest != i) {
            std::swap(arr[i], arr[largest]);

            // Recursively heapify the affected sub-tree 
            heapify(arr, n, largest, pred);
        }
    }
}

namespace sort
{
    template<typename T, typename Predicate = std::less<T>>
    void heap_sort(T* arr, size_t size, Predicate&& pred = Predicate()) {
        using namespace heap_sort_impl;
        const int n = static_cast<int>(size);

        // Build heap (rearrange array) 
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i, pred);
        }

        // One by one extract an element from heap 
        for (int i = n - 1; i >= 0; i--) {
            // Move current root to end 
            std::swap(arr[0], arr[i]);

            // call max heapify on the reduced heap 
            heapify(arr, i, 0, pred);
        }
    }
}
