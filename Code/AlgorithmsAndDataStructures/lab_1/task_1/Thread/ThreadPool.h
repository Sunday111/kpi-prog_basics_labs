#pragma once

#include <functional>
#include <mutex>
#include <thread>

template<typename JobSignature>
class ThreadPool;

template<typename JobResult>
class ThreadPool<JobResult()>
{
public:
    using Job = std::function<JobResult()>;

public:
    ThreadPool(size_t threadsCount = 0) :
        m_desiredThreadsCount(threadsCount)
    {
    }

    void AddJob(Job job) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_pending.push_back(std::move(job));
    }

    void Start() {
        size_t pool_size = m_desiredThreadsCount;
        if (pool_size == 0) {
            size_t pool_size = std::thread::hardware_concurrency();
            pool_size = std::max(size_t(1), pool_size);
        }
        //pool_size = std::min(m_pending.size(), pool_size);

        for (size_t i = 0; i < pool_size; ++i) {
            m_workers.push_back(std::thread([this]() {
                auto get_job = [&]() {
                    std::optional<Job> result;
                    std::lock_guard<std::mutex> guard(m_mutex);
                    if (!m_pending.empty()) {
                        result = std::move(m_pending.back());
                        m_pending.pop_back();
                    }

                    return result;
                };

                auto submit_result = [&](JobResult result) {
                    std::lock_guard<std::mutex> guard(m_mutex);
                    m_results.push_back(std::move(result));
                };

                auto need_stop = [&]() {
                    std::lock_guard<std::mutex> guard(m_mutex);
                    return m_stopRequested;
                };

                while (true) {
                    std::optional<Job> job = get_job();
                    if (job.has_value()) {
                        JobResult jobResult = job.value()();
                        submit_result(std::move(jobResult));
                    }
                    else {
                        if (need_stop()) {
                            break;
                        }

                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(100ms);
                    }
                }
            }));
        }
    }

    void ForEachResult(std::function<void(JobResult&)> callback) {
        for (JobResult& result: m_results) {
            callback(result);
        }
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_stopRequested = true;
        }

        for (auto& worker : m_workers) {
            worker.join();
        }
    }

private:
    bool m_stopRequested = false;
    size_t m_desiredThreadsCount = 0;
    std::mutex m_mutex;
    std::vector<Job> m_pending;
    std::vector<std::thread> m_workers;
    std::vector<JobResult> m_results;
};