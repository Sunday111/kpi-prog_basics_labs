// quick sort
// merge sort
// pyramid sort

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>
#include "sort/bubble_sort.h"
#include "sort/counting_sort.h"
#include "sort/quick_sort.h"
#include "sort/merge_sort.h"
#include "sort/heap_sort.h"

template<typename T>
T* get_vector_data(std::vector<T>& vec) {
    return vec.empty() ? nullptr : &vec[0];
}

template<typename T, typename Predicate>
class sort_functor
{
public:
    virtual std::string_view get_name() const = 0;
    virtual void update_cache(T* array, size_t size) = 0;
    virtual void sort(T* array, size_t size) = 0;
};

template<typename T, typename Predicate>
class bubble_sort_functor :
    public sort_functor<T, Predicate>
{
public:
    virtual std::string_view get_name() const override final {
        return "Bubble Sort";
    }

    virtual void update_cache(T*, size_t) override final {

    }

    virtual void sort(T* array, size_t size) override final {
        sort::bubble_sort<T, Predicate>(array, size);
    }
};

template<typename T, typename Predicate>
class counting_sort_functor :
    public sort_functor<T, Predicate>
{
private:
    using limits = std::numeric_limits<T>;
public:
    virtual std::string_view get_name() const override final {
        return "Counting Sort";
    }

    virtual void update_cache(T* array, size_t size) override final {
        m_min = limits::max();
        m_max = limits::lowest();
        for (size_t i = 0; i < size; ++i) {
            m_min = std::min(m_min, array[i]);
            m_max = std::max(m_max, array[i]);
        }
        m_counts.resize(m_max - m_min + 1);
    }

    virtual void sort(T* array, size_t size) override final {
        sort::counting_sort<T>(array, get_vector_data(m_counts), size, m_min, m_max);
    }

private:
    T m_min = 0;
    T m_max = 0;
    std::vector<T> m_counts;
};

template<typename T, typename Predicate>
class quick_sort_functor :
    public sort_functor<T, Predicate>
{
public:
    virtual std::string_view get_name() const override final {
        return "Quick Sort";
    }

    virtual void update_cache(T*, size_t) override final {

    }

    virtual void sort(T* array, size_t size) override final {
        sort::quick_sort<T, Predicate>(array, size);
    }
};

template<typename T, typename Predicate>
class merge_sort_functor :
    public sort_functor<T, Predicate>
{
private:
    using limits = std::numeric_limits<T>;
public:
    virtual std::string_view get_name() const override final {
        return "Merge Sort";
    }

    virtual void update_cache(T* array, size_t size) override final {
        m_cache.resize(size);
    }

    virtual void sort(T* array, size_t size) override final {
        sort::merge_sort<T, Predicate>(array, get_vector_data(m_cache), size);
    }

private:
    std::vector<T> m_cache;
};

template<typename T, typename Predicate>
class heap_sort_functor :
    public sort_functor<T, Predicate>
{
public:
    virtual std::string_view get_name() const override final {
        return "Heap Sort";
    }

    virtual void update_cache(T*, size_t) override final {

    }

    virtual void sort(T* array, size_t size) override final {
        sort::heap_sort<T, Predicate>(array, size);
    }
};

template<typename T, typename Predicate>
class sort_functor_adapter
{
public:
    sort_functor_adapter(sort_functor<T, Predicate>& functor) :
        m_functor(functor)
    {}

    std::string_view get_name() const {
        return m_functor.get_name();
    }

    void update_cache(std::vector<T>& vec) {
        m_functor.update_cache(get_vector_data(vec), vec.size());
    }

    void sort(std::vector<T>& vec) {
        m_functor.sort(get_vector_data(vec), vec.size());
    }

private:
    sort_functor<T, Predicate>& m_functor;
};

int main() {
    using T = int;
    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::nanoseconds;
    using sort_predicate = std::greater<T>;
    using functor_adapter = sort_functor_adapter<T, sort_predicate>;

    std::random_device rd;
    std::mt19937 gen(rd());

    constexpr size_t collectionSize = 10000;
    std::ostream& output = std::cout;

    auto shuffle = [&gen](std::vector<T>& data) {
        std::shuffle(data.begin(), data.end(), gen);
    };

    auto generate_unique = [](std::vector<T>& out, size_t count) {
        out.reserve(out.size() + count);
        for (T i = 0; i < count; ++i) {
            out.push_back(i);
        }
    };

    auto generate_random = [&gen](std::vector<T>& out, size_t count, T min, T max) {
        out.reserve(out.size() + count);
        std::uniform_int_distribution<T> duplicatesDistribution(min, max);
        std::generate_n(std::back_inserter(out), count, [&]() {
            return duplicatesDistribution(gen);
        });
    };

    auto presort_part = [&gen](std::vector<T>& out, size_t count, auto pred) {
        assert(count <= out.size());
        std::sort(out.begin(), out.end(), pred);
        std::shuffle(out.begin() + count, out.end(), gen);
    };

    std::vector<T> merge_sort_cache;
    std::vector<T> input_data_cache;
    std::vector<T> algorithm_cache;

    std::vector<std::unique_ptr<sort_functor<T, sort_predicate>>>  functors;
    functors.push_back(std::make_unique<bubble_sort_functor<T, sort_predicate>>());
    functors.push_back(std::make_unique<counting_sort_functor<T, sort_predicate>>());
    functors.push_back(std::make_unique<merge_sort_functor<T, sort_predicate>>());
    functors.push_back(std::make_unique<quick_sort_functor<T, sort_predicate>>());
    functors.push_back(std::make_unique<heap_sort_functor<T, sort_predicate>>());

    auto print = [&output](auto... args) {
        (output << ... << args);
    };

    auto println = [&print](auto... args) {
        print(args..., '\n');
    };

    auto get_process_duration = [](auto&& fn) {
        auto t1 = clock::now();
        fn();
        auto t2 = clock::now();
        return std::chrono::duration_cast<duration>(t2 - t1);
    };

    auto print_table_header = [&]() {
        print("N,");
        for (auto& functor : functors) {
            print(functor->get_name(), ',');
        }
        println();
    };

    {
        println("Check that sorting functions work in the same way as std::sort");
        input_data_cache.clear();
        generate_random(input_data_cache, collectionSize, 0, static_cast<T>(collectionSize));

        std::vector<T> std_sorted = input_data_cache;
        presort_part(std_sorted, std_sorted.size(), sort_predicate{});

        for (auto& functor : functors) {
            functor_adapter f{*functor };
            print("   ", functor->get_name(), ": ");
            algorithm_cache = input_data_cache;
            f.update_cache(algorithm_cache);
            f.sort(algorithm_cache);
            if (algorithm_cache == std_sorted) {
                println("OK");
            } else {
                println("FAILED");
            }
        }
        println();
    }

    {
        println("Sorting time as function of collection size");
        print_table_header();

        const size_t steps_count = 100;
        const size_t step_size = collectionSize / steps_count;
        static_assert((collectionSize % steps_count) == 0);
    
        // Prepare data for the whole test
        input_data_cache.clear();
        generate_random(input_data_cache, collectionSize, 0, static_cast<T>(collectionSize));
    
        for (size_t i = 1; i <= steps_count; ++i) {
            const size_t test_size = step_size * i;
            print(test_size, ',');
            for (auto& functor : functors) {
                // copy first n from generated in cache
                algorithm_cache.resize(test_size);
                std::copy_n(input_data_cache.begin(), test_size, algorithm_cache.begin());
                functor_adapter f{ *functor };
                f.update_cache(algorithm_cache);
                const duration time = get_process_duration([&] {
                    f.sort(algorithm_cache);
                });
    
                print(time.count(), ',');
            }
            println();
        }
        println();
    }

    {
        println("Sorting time as function of sorted part size");
        print_table_header();

        const size_t steps_count = 100;
        const size_t step_size = collectionSize / steps_count;
        static_assert((collectionSize % steps_count) == 0);

        // Prepare common test data
        input_data_cache.clear();
        generate_random(input_data_cache, collectionSize, 0, static_cast<T>(collectionSize));

        for (size_t i = 1; i <= steps_count; ++i) {
            const size_t test_size = step_size * i;
            auto percents = (100.f * i) / steps_count;
            print(percents, ',');
            presort_part(input_data_cache, test_size, sort_predicate{});
            for (auto& functor : functors) {
                algorithm_cache = input_data_cache;
                functor_adapter f{ *functor };
                f.update_cache(algorithm_cache);
                const duration time = get_process_duration([&] {
                    f.sort(algorithm_cache);
                });

                print(time.count(), ',');
            }
            println();
        }
        println();
    }

    {
        println("Sorting time as function of duplicated elements percent");
        print_table_header();

        const size_t steps_count = 100;
        const size_t step_size = collectionSize / steps_count;
        static_assert((collectionSize % steps_count) == 0);
    
        for (size_t i = 1; i <= steps_count; ++i) {
            const size_t duplicates = step_size * i;
            const size_t unique = std::max(size_t{ 1 }, collectionSize - duplicates);
            auto percents = (100.f * duplicates) / collectionSize;
            print(percents, ',');
            input_data_cache.clear();
            generate_unique(input_data_cache, unique);
            generate_random(input_data_cache, duplicates, 0, static_cast<T>(unique) - 1);
            shuffle(input_data_cache);
            for (auto& functor : functors) {
                // copy first n from generated in cache
                algorithm_cache = input_data_cache;
                functor_adapter f{ *functor };
                f.update_cache(algorithm_cache);
                const duration time = get_process_duration([&] {
                    f.sort(algorithm_cache);
                });
    
                print(time.count(), ',');
            }
            println();
        }
        println();
    }

    system("pause");
    return 0;
}