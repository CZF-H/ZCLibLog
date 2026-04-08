// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_ASYNC_HPP
#define ZCLIBLOG_LOGGER_ASYNC_HPP

#include <algorithm>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <mutex>

// ReSharper disable CppUnusedIncludeDirective

#include "inside/logger_precompile.hpp"
#include "inside/logger_types.hpp"

#include "inside/logger_constants.hpp"

#include "logger_configurations.h"
#if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
#include "formatters/csnprintf.hpp"
#endif

#if defined(ZCLibLog_HAS_FORMAT) && ZCLIBLOG_LOGGER_CONFIGURATIONS_ENABLE_CXX20_FORMAT
#define ZCLibLog_USE_FORMAT
#include <format>
#endif
#include <type_traits>

#if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
#ifndef ZCLibLog_MUTEX
#if ZCLibLog_CPP >= 17
#include <shared_mutex>
#define ZCLibLog_MUTEX std::shared_mutex
#else
#define ZCLibLog_MUTEX ZCLibLog_MUTEX
#endif
#endif
#endif

// ReSharper enable CppUnusedIncludeDirective

namespace ZCLibLog {
    namespace details {
        class ThreadPool {
        public:
            explicit ThreadPool(const size_t numThreads) : stop(false) {
                for (size_t i = 0; i < numThreads; ++i) {
                    workers.emplace_back([this] {
                        while (true) {
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> lock(mtx);
                                cv.wait(lock, [this] { return stop || !tasks.empty(); });
                                if (stop && tasks.empty()) return;
                                task = std::move(tasks.front());
                                tasks.pop();
                            }
                            task();
                        }
                    });
                }
            }

            ~ThreadPool() {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    stop = true;
                }
                cv.notify_all();
                for (auto& t : workers) t.join();
            }

            void submit(std::function<void()> task) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    tasks.push(std::move(task));
                }
                cv.notify_one();
            }

        private:
            std::vector<std::thread> workers;
            std::queue<std::function<void()>> tasks;
            std::mutex mtx;
            std::condition_variable cv;
            std::atomic<bool> stop;
        };
    }

    static details::ThreadPool LoggerAsync_ThreadPool{ZCLIBLOG_LOGGER_CONFIGURATIONS_ASYNC_THREAD_NUM};

    template <
        typename Formatter
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
        = formatters::csnprintf
        #endif
    >
    class LoggerAsync {
        // ReSharper disable CppUseTypeTraitAlias
        static_assert(is_format_api<Formatter>::value,
                  "Formatter must be format_api");
        // ReSharper enable CppUseTypeTraitAlias
    protected:
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
        mutable ZCLibLog_MUTEX m_mutex;
        #endif

        using executor_pair = std::pair<size_t, executor>;

        std::string m_name;
        std::vector<executor_pair> m_executors;
        size_t m_nextID{};

        LogLevelCfg m_config;
    public:
        ZCLibLog_NODISCARD const std::string& name() const noexcept {
            return m_name;
        }

        ZCLibLog_NODISCARD LogLevelCfg& config() noexcept {
            return m_config;
        }

        ZCLibLog_NODISCARD bool be_executable(const LogLevel level) const noexcept {
            return m_config.min_level <= level && level <= m_config.max_level;
        }

        void execute(const std::shared_ptr<std::string>& message, const LogLevel level) const {
            if (!message->empty()) {
                LoggerAsync_ThreadPool.submit(
                        [this, message, level] {
                            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
                            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
                            #endif
                            // ReSharper disable once CppUseStructuredBinding
                            // ReSharper disable once CppUseElementsView
                            for (const auto& the_executor_pair : m_executors) {
                                the_executor_pair.second->do_execute(*message, level);
                            }
                        }
                    );
            }
        }

        ZCLibLog_NODISCARD bool has_executor() const {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            return !m_executors.empty();
        }

        size_t bind_executor(const executor& ex) {
            if (!ex) throw std::invalid_argument("executor is nullptr");
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.emplace_back(m_nextID, ex);
            return m_nextID++;
        }

        void debind_executor(size_t id) {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.erase(
                std::remove_if(
                    m_executors.begin(),
                    m_executors.end(),
                    [id](const executor_pair& p) { return p.first == id; }
                ),
                m_executors.end()
            );
        }

        void clear_executors() {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.clear();
            m_nextID = {};
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        LoggerAsync(
            std::string name,
            const std::initializer_list<executor>& executor_ptrs = {},
            const LogLevelCfg config = {}
        ) : m_name(std::move(name)),
            m_config(config) {
            for (const auto& executor_ptr : executor_ptrs) {
                bind_executor(executor_ptr);
            }
        }

        class Tag {
            ZCLibLog_NODISCARD LogPack get_log_pack() const {
                const auto now = std::chrono::system_clock::now();
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count();

                LogPack p;
                p.name = &m_logger->name();
                p.level = m_level;
                p.time = ms;

                return p;
            }

            ZCLibLog_NODISCARD bool check_executable() const {
                if (
                    m_logger->has_executor() &&
                    m_logger->be_executable(level())
                ) return true;
                return {};
            }
        protected:
            const LoggerAsync* const m_logger{};
            const LogLevel m_level{};
        public:
            ZCLibLog_NODISCARD const LogLevel& level() const noexcept {
                return m_level;
            }

            Tag(const LoggerAsync* const logger, const LogLevel level) : m_logger(logger),
                                                                         m_level(level) {}

            template <typename Fmt, typename... Args>
            #ifdef ZCLibLog_HAS_CONSTRAINTS
            requires is_format_api<Formatter, format_apis::traditional>::value
            #endif
            void operator()(Fmt&& fmt, Args&&... args) const {
                if (!check_executable()) return;
                auto Formatted = std::make_shared<std::string>(
                    Formatter::do_format(
                        get_log_pack(),
                        std::forward<Fmt>(fmt),
                        std::forward<Args>(args)...
                    )
                );
                m_logger->execute(Formatted, level());
            }

            #ifdef ZCLibLog_USE_FORMAT
            template <typename... Args>
            #ifdef ZCLibLog_HAS_CONSTRAINTS
            requires is_format_api<Formatter, format_apis::stdcxx20>::value
            #endif
            void operator()(std::format_string<Args...>&& fmt, Args&&... args) const {
                if (!check_executable()) return;
                auto Formatted = std::make_shared<std::string>(
                    Formatter::do_format(
                        get_log_pack(),
                        std::forward<std::format_string<Args...>&&>(fmt),
                        std::forward<Args>(args)...
                    )
                );
                m_logger->execute(Formatted, level());
            }
            #endif
        };

        Tag ALL{this, LogLevel::ALL};
        Tag TRACE{this, LogLevel::TRACE};
        Tag DEBUG{this, LogLevel::DEBUG};
        Tag INFO{this, LogLevel::INFO};
        Tag WARN{this, LogLevel::WARN};
        Tag ERROR{this, LogLevel::ERROR};
        Tag FATAL{this, LogLevel::FATAL};
    };
}

#endif //ZCLIBLOG_LOGGER_ASYNC_HPP
