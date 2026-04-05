// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_SYNC_HPP
#define ZCLIBLOG_LOGGER_SYNC_HPP

#include <algorithm>
#include <vector>
#include <string>

#include "inside/logger_precompile.hpp"
#include "inside/logger_types.hpp"

// ReSharper disable CppUnusedIncludeDirective
#include "inside/logger_constants.hpp"
// ReSharper enable CppUnusedIncludeDirective

#include "logger_configurations.h"
#if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
#include "formatters/csnprintf.hpp"
#endif

#if defined(ZCLibLog_HAS_FORMAT) && ZCLIBLOG_LOGGER_CONFIGURATIONS_ENABLE_CXX20_FORMAT
#define ZCLibLog_USE_FORMAT
#include <format>
#endif
#include <type_traits>

namespace ZCLibLog {
    template <
        typename Formatter
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
        = formatters::csnprintf
        #endif
    >
    class LoggerSync {
    protected:
        using executor_pair = std::pair<size_t, executor>;

        std::string m_name;
        LogLevel m_level;
        size_t m_nextID{};

        std::vector<executor_pair> m_executors;

    public:
        ZCLibLog_NODISCARD const std::string& name() const noexcept {
            return m_name;
        }

        ZCLibLog_NODISCARD LogLevel& level() noexcept {
            return m_level;
        }

        void execute(const std::string& message, const LogLevel level) const {
            if (!message.empty()) {
                // ReSharper disable once CppUseElementsView
                // ReSharper disable once CppUseStructuredBinding
                for (const auto& the_executor_pair : m_executors) {
                    the_executor_pair.second(message, level);
                }
            }
        }

        ZCLibLog_NODISCARD bool has_executor() const {
            return !m_executors.empty();
        }

        size_t bind_executor(executor ex) {
            m_executors.emplace_back(m_nextID, std::move(ex));
            return m_nextID++;
        }

        void debind_executor(size_t id) {
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
            m_executors.clear();
            m_nextID = {};
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        LoggerSync(
            std::string name,
            const std::initializer_list<executor>& executors = {},
            const LogLevel level = LogLevel_ALL
        ) : m_name(std::move(name)),
            m_level(level) {
            for (const auto& executor : executors) {
                bind_executor(executor);
            }
        }

        class Tag {
            [[nodiscard]] LogPack get_log_pack() const {
                const auto now = std::chrono::system_clock::now();
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count();

                LogPack p;
                p.module = &m_logger->name();
                p.level = m_level;
                p.time = ms;

                return p;
            }
        protected:
            const LoggerSync* const m_logger{};
            const LogLevel m_level{};

        public:
            ZCLibLog_NODISCARD const LogLevel& level() const noexcept {
                return m_level;
            }

            Tag(const LoggerSync* const logger, const LogLevel level) : m_logger(logger),
                                                                        m_level(level) {}

            template <typename Fmt, typename... Args>
            #ifdef ZCLibLog_USE_FORMAT
            requires (!std::is_constructible_v<std::format_string<Args...>, Fmt&&>)
            #endif
            void operator()(Fmt&& fmt, Args&&... args) const {
                if (!m_logger->has_executor()) return;
                const std::string Formatted = Formatter::do_format(
                    get_log_pack(),
                    std::forward<Fmt>(fmt),
                    std::forward<Args>(args)...
                );
                m_logger->execute(Formatted, level());
            }

            #ifdef ZCLibLog_USE_FORMAT
            template <typename... Args>
            void operator()(std::format_string<Args...> fmt, Args&&... args) const {
                if (!m_logger->has_executor()) return;
                const std::string Formatted = Formatter::do_format(
                    get_log_pack(),
                    fmt,
                    std::forward<Args>(args)...
                );
                m_logger->execute(Formatted, level());
            }
            #endif
        };

        Tag ALL{this, LogLevel_ALL};
        Tag TRACE{this, LogLevel_TRACE};
        Tag DEBUG{this, LogLevel_DEBUG};
        Tag INFO{this, LogLevel_INFO};
        Tag WARN{this, LogLevel_WARN};
        Tag ERROR{this, LogLevel_ERROR};
        Tag FATAL{this, LogLevel_FATAL};
        Tag NONE{this, LogLevel_NONE};
    };
}

#endif //ZCLIBLOG_LOGGER_SYNC_HPP
