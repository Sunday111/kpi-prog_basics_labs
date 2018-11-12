#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>

template<typename T, typename Cmp = std::less<T>>
void bubble_sort(T* arr, size_t size, Cmp cmp = Cmp{}) {
    size_t n = size;
    size_t nn;
    do 
    {
        nn = 0;
        for (size_t i = 1; i < n; ++i) {
            if (cmp(arr[i], arr[i-1])) {
                std::swap(arr[i - 1], arr[i]);
                nn = i;
            }
        }
        n = nn;
    } while (n > 1);
}

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
        for(size_t j = 0; j < count; ++j) {
            *array++ = i;
        }
    }
}

int main() {
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::nanoseconds;
    using T = int;

    T minValue = 0;
    T maxValue = 10000;
    constexpr size_t maxCollectionSize = 10000;
    std::ostream& output = std::cout;

    std::random_device rd;
    std::mt19937 gen(rd());

    auto print = [&output](auto... args) {
        (output << ... << args);
    };

    auto println = [&print](auto... args) {
        print(args..., '\n');
    };

    auto get_vector_data = [](std::vector<T>& array) {
        return array.empty() ? (T*)(0) : &array[0];
    };

    // Unify sorting algorithm interface
    auto bubble = [&](std::vector<T>& array) {
        bubble_sort(get_vector_data(array), array.size());
    };

    auto counting = [&](std::vector<T>& array) {
        counting_sort(get_vector_data(array), array.size(), minValue, maxValue);
    };

    auto get_process_duration = [](auto& f) {
        auto t1 = Clock::now();
        f();
        auto t2 = Clock::now();
        return std::chrono::duration_cast<Duration>(t2 - t1).count();
    };

    struct CollectionParameters
    {
        size_t collectionSize;
        size_t sortedCount = 0;
        size_t uniqueCount = 0;
    };

    auto configure_data = [&](std::vector<T>& out, const CollectionParameters& params) {
        out.resize(params.collectionSize);

        const auto uniques = params.uniqueCount == 0 ? params.collectionSize : params.uniqueCount;

        // fill first part with unique values
        T i = 0;
        while (i < uniques) {
            out[i] = i;
            ++i;
        }

        if (params.uniqueCount) {
            // fill another part with random duplicates
            std::uniform_int_distribution<> duplicatesDist(0, static_cast<T>(uniques - 1));
            while (i < params.collectionSize) {
                out[i] = duplicatesDist(gen);
                ++i;
            }

            std::sort(out.begin(), out.end());
        }

        // Shuffle everything after sorted part
        std::shuffle(out.begin() + params.sortedCount, out.end(), gen);
    };

    std::vector<int> initValues;
    std::vector<int> cache;

    {
        println("Test that algorithm even works");
        CollectionParameters collectionParameters;
        collectionParameters.collectionSize = maxCollectionSize;
        collectionParameters.sortedCount = 0;
        configure_data(initValues, collectionParameters);

        auto test = initValues;
        std::sort(test.begin(), test.end());

        cache = initValues;
        bubble(cache);
        if (cache != test) {
            println("    Bubble Sort doesn't work");
            return 1;
        }
        println("    Bubble Sort is OK");

        cache = initValues;
        counting(cache);
        if (cache != test) {
            println("    Counting Sort doesn't work");
            return 1;
        }
        println("    Counting Sort is OK");
        println();
    }

    {
        println("Sorting speed as function of collection size");
        CollectionParameters collectionParameters;
        collectionParameters.collectionSize = maxCollectionSize;
        collectionParameters.sortedCount = 0;
        configure_data(initValues, collectionParameters);
    
        auto get_sorting_duration = [&](auto& sort_fn, size_t n) {
            cache.resize(n);
            std::copy_n(initValues.begin(), n, cache.begin());
            return get_process_duration([&sort_fn, &cache]() { sort_fn(cache); });
        };

        constexpr size_t steps = 100;
        constexpr size_t step = maxCollectionSize / steps;
        for (size_t collectionSize = 0; collectionSize <= maxCollectionSize; collectionSize += step) {
            auto bubble_time = get_sorting_duration(bubble, collectionSize);
            auto counting_time = get_sorting_duration(counting, collectionSize);
            println(collectionSize, ',', bubble_time, ',', counting_time);
        }
        println();
    }
    
    {
        println("Sorting speed as function of presorted elements count");
        CollectionParameters collectionParameters;
        collectionParameters.collectionSize = maxCollectionSize;
    
        constexpr size_t stepsCount = 100;
        auto get_sorting_duration = [&](auto& sort_fn) {
            cache = initValues;
            return get_process_duration([&sort_fn, &cache]() { sort_fn(cache); });
        };
    
        for (size_t i = 0; i < stepsCount; ++i) {
            collectionParameters.sortedCount = i * maxCollectionSize / stepsCount;
            configure_data(initValues, collectionParameters);
            auto bubble_time = get_sorting_duration(bubble);
            auto counting_time = get_sorting_duration(counting);
            auto percents = (100.f / stepsCount) * i;
            println(percents, ',', bubble_time, ',', counting_time);
        }
        println();
    }

    {
        println("Sorting speed as function of duplicated elements count");
        CollectionParameters collectionParameters;
        collectionParameters.collectionSize = maxCollectionSize;
        collectionParameters.sortedCount = 0;

        auto get_sorting_duration = [&](auto& sort_fn) {
            cache = initValues;
            return get_process_duration([&sort_fn, &cache]() { sort_fn(cache); });
        };

        constexpr size_t steps = 100;
        for (size_t i = 0; i < steps; ++i) {
            const size_t duplicates = i * maxCollectionSize / steps;
            collectionParameters.uniqueCount = maxCollectionSize - duplicates;
            maxValue = static_cast<T>(collectionParameters.uniqueCount);
            configure_data(initValues, collectionParameters);
            auto bubble_time = get_sorting_duration(bubble);
            auto counting_time = get_sorting_duration(counting);
            println(i, ',', bubble_time, ',', counting_time);
        }
        println();
    }

    return 0;
}