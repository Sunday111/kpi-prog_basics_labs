#pragma once

#include <chrono>
#include <mutex>
#include <ostream>
#include <type_traits>
#include <iomanip>

namespace Log::detail
{
    class Empty {};

    class ThreadSafeLogger
    {
    protected:
        std::mutex m_mutex;
    };

    template<bool thread_safe>
    using LoggerBase = std::conditional_t<thread_safe, ThreadSafeLogger, Empty>;
}

namespace Log
{
    template
    <
        typename Clock = std::chrono::high_resolution_clock,
        bool thread_safe = true
    >
    class Logger : protected detail::LoggerBase<thread_safe>
    {
    protected:
        class LockGuard
        {
        public:
            LockGuard(Logger& logger) :
                m_logger(logger)
            {
                if constexpr (thread_safe) {
                    m_logger.m_mutex.lock();
                }
            }

            ~LockGuard() {
                if constexpr (thread_safe) {
                    m_logger.m_mutex.unlock();
                }
            }

            Logger& m_logger;
        };

    public:
        Logger(std::ostream& output) :
            m_output(output),
            m_startTime(Clock::now())
        {
        }

        template<typename... Args>
        void Write(Args... args) {
            LockGuard lock(*this);
            WriteTime();
            (m_output << ... << args);
            m_output << '\n';
        }



    protected:
        void WriteTime() {
            using namespace std::chrono;
            const auto dt = Clock::now() - m_startTime;
            const auto hours = duration_cast<std::chrono::hours>(dt).count();
            const auto minutes = duration_cast<std::chrono::minutes>(dt).count() - hours * 60;
            const auto seconds = duration_cast<std::chrono::seconds>(dt).count() - minutes * 60;
            const auto milliseconds = duration_cast<std::chrono::milliseconds>(dt).count() - seconds * 1000;
            m_output << '[' << std::setw(2) << hours;
            m_output << ':' << std::setw(2) << minutes;
            m_output << ':' << std::setw(2) << seconds;
            m_output << ':' << std::setw(3) << milliseconds;
            m_output << "] ";
        }

    private:
        std::ostream& m_output;
        std::chrono::time_point<Clock> m_startTime;
        std::mutex m_mutex;
    };
}
