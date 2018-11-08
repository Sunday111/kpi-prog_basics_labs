#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template<typename Logger>
class ThreadPool
{
public:
    using Task = std::function<void()>;

private:
    struct thread_id
    {
        size_t value;
    };

public:
    ThreadPool(Logger* logger = nullptr) :
        m_logger(logger)
    {
    }

    void AddTask(Task task) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_pending.push(std::move(task));
    }

    void Start() {
        using namespace std::chrono_literals;
        m_mainId = std::this_thread::get_id();

        size_t threads_count = m_desiredThreadsCount;
        if (threads_count == 0) {
            threads_count = std::thread::hardware_concurrency();
            threads_count = std::max(size_t{ 1 }, threads_count);
        }
        m_done = false;

        for (size_t i = 0; i < threads_count; ++i) {
            m_workers.emplace_back(std::thread([&, id = thread_id{i}]() {
                auto log = [&] (auto... args) {
                    Log(id, args...);
                };

                auto get_task = [&]() {
                    std::optional<Task> result;
                    std::lock_guard<std::mutex> guard(m_mutex);
                    if (!m_pending.empty()) {
                        result = std::move(m_pending.front());
                        m_pending.pop();
                    }

                    return result;
                };

                auto need_stop = [&]() {
                    std::lock_guard<std::mutex> guard(m_mutex);
                    return m_done;
                };

                while (true) {
                    std::optional<Task> task = get_task();
                    if (task.has_value()) {
                        task.value()();
                    }
                    else {
                        if (need_stop()) {
                            break;
                        }

                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(100ms);
                    }
                }

                log("Exit");
            }));
        }
    }

    void StopAndWait() {
        Log("Stop and wait begin");
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_done = true;
        }

        for (auto& worker : m_workers) {
            worker.join();
        }
        m_workers.clear();
        Log("Stop and wait end");
    }

    void Wait() {
        using namespace std::chrono_literals;
        Log("Wait begin");
        while (true) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_pending.empty()) {
                    break;
                }
            }
            std::this_thread::sleep_for(100ms);
        }
        Log("Wait end");
    }

    void SetDesiredThreadsCount(size_t threadsCount) {
        m_desiredThreadsCount = threadsCount;
    }

private:
    template<typename... Args>
    void Log(Args&&... args) {
        if (m_logger) {
            m_logger->Write("[ThreadPool] ", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void Log(thread_id id, Args&&... args) {
        if (m_logger) {
            const auto this_thread_id = std::this_thread::get_id();
            if (this_thread_id == m_mainId) {
                m_logger->Write("[ThreadPool] ", std::forward<Args>(args)...);
            }
            else {
                m_logger->Write("[ThreadPool (", id.value, ")] ", std::forward<Args>(args)...);
            }
        }
    }

private:
    Logger* m_logger = nullptr;

    std::mutex m_mutex;
    std::queue<Task> m_pending;
    std::vector<std::thread> m_workers;
    std::thread::id m_mainId;
    size_t m_desiredThreadsCount = 0;
    bool m_done = false;
};
