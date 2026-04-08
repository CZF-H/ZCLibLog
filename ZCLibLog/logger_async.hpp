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

#if defined(ZCLibLog_HAS_STD_FORMAT) && ZCLIBLOG_LOGGER_CONFIGURATIONS_ENABLE_CXX20_FORMAT
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
    /**
     * @namespace details
     * @brief 内部使用的工具
     */
    namespace details {
        /**
         * @class ThreadPool
         * @brief 内部使用的线程池
         */
        class ThreadPool {
        public:
            /**
             * @brief 构造线程池
             * @param numThreads 线程数量（默认1）
             */
            explicit ThreadPool(const size_t numThreads = 1) : stop(false) {
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

            /// @brief 析构线程池
            ~ThreadPool() {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    stop = true;
                }
                cv.notify_all();
                for (auto& t : workers) t.join();
            }

            /**
             * @brief 提交任务到线程池
             * @param task 要提交的任务
             */
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

    /// @brief 全局异步线程池
    static details::ThreadPool LoggerAsync_ThreadPool{ZCLIBLOG_LOGGER_CONFIGURATIONS_ASYNC_THREAD_NUM};

    /**
     * @class LoggerAsync
     * @brief 异步日志器
     * @tparam Formatter 绑定的格式化器
     */
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
        /// @brief 获取日志器的名字
        ZCLibLog_NODISCARD const std::string& name() const noexcept {
            return m_name;
        }

        /// @brief 获取并可修改日志器的等级配置
        ZCLibLog_NODISCARD LogLevelCfg& config() noexcept {
            return m_config;
        }

        /**
         * @brief 调用执行器处理日志信息和等级
         * @param message 日志信息
         * @param level 日志等级
         */
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

        /**
         * @brief 检查等级是否可执行
         * @param level 要检查的等级
         * @return 是否可执行
         */
        ZCLibLog_NODISCARD bool be_executable(const LogLevel level) const noexcept {
            return m_config.min_level <= level && level <= m_config.max_level;
        }

        /// @brief 判断是否有执行器
        ZCLibLog_NODISCARD bool has_executor() const {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            return !m_executors.empty();
        }

        /**
         * @brief 绑定执行器
         * @param ex 执行器常量引用
         * @return 执行器在日志器的id
         */
        size_t bind_executor(const executor& ex) {
            if (!ex) throw std::invalid_argument("executor is nullptr");
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.emplace_back(m_nextID, ex);
            return m_nextID++;
        }

        /**
         * @brief 解绑执行器
         * @param id 执行器在日志器的id
         */
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

        /// @brief 清空所有执行器
        void clear_executors() {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.clear();
            m_nextID = {};
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        /**
         * @brief 构造同步日志器
         * @param name 日志器名字
         * @param executor_ptrs 日志器预绑定执行器
         * @param config 日志器等级配置
         */
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

        /**
         * @class Tag
         * @brief Tag为日志标签，用于输出
         */
        class Tag {
            /// @brief 获取当前的日志信息包
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

            /// @brief 检查是否可执行
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
            /// @brief 获取当前Tag的等级
            ZCLibLog_NODISCARD const LogLevel& level() const noexcept {
                return m_level;
            }

            /**
             * @brief 构造Tag
             * @param logger 日志器指针（引用非拥有）
             * @param level 当前Tag的日志等级
             */
            Tag(const LoggerAsync* const logger, const LogLevel level) : m_logger(logger),
                                                                         m_level(level) {}

            /**
             * @brief 输出日志
             * @tparam Fmt 格式字符串的类型（自动推导）
             * @tparam Args 格式化可变参数类型（自动推导）
             * @param fmt 格式字符串
             * @param args 格式化可变参数
             */
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
            /**
             * @brief 输出日志
             * @tparam Args 格式化可变参数类型（自动推导）
             * @param fmt 格式字符串（必须为字面量）
             * @param args 格式化可变参数
             */
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

        /// @brief ALL级别Tag
        Tag ALL{this, LogLevel::ALL};
        /// @brief TRACE级别Tag
        Tag TRACE{this, LogLevel::TRACE};
        /// @brief DEBUG级别Tag
        Tag DEBUG{this, LogLevel::DEBUG};
        /// @brief INFO级别Tag
        Tag INFO{this, LogLevel::INFO};
        /// @brief WARN级别Tag
        Tag WARN{this, LogLevel::WARN};
        /// @brief ERROR级别Tag
        Tag ERROR{this, LogLevel::ERROR};
        /// @brief FATAL级别Tag
        Tag FATAL{this, LogLevel::FATAL};
    };
}

#endif //ZCLIBLOG_LOGGER_ASYNC_HPP
