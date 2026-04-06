// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/5.
//

#ifndef ZCLIBLOG_FORMATTERS_FORMAT_HPP
#define ZCLIBLOG_FORMATTERS_FORMAT_HPP

#include <format>
#include <chrono>
#include "../inside/logger_types.hpp"

// NOLINTNEXTLINE
namespace ZCLibLog::formatters {
    struct format : format_apis::stdcxx20 {
        template <typename... Args>
        static std::string do_format(FLogPack pack, std::format_string<Args...>&& fmt, Args&&... args) {

            std::string f_msg;
            if (sizeof...(args) == 0) {
                f_msg = fmt.get();
            }
            else {
                try {
                    f_msg = std::format(std::forward<std::format_string<Args...>&&>(fmt), std::forward<Args&&>(args)...);
                } catch (const std::format_error&) {
                    return {};
                }
            }

            const auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(pack.time));

            const char* level_str;
            switch (pack.level) {
                case LogLevel_TRACE: level_str = "[TRACE]";
                    break;
                case LogLevel_DEBUG: level_str = "[DEBUG]";
                    break;
                case LogLevel_INFO: level_str = "[INFO]";
                    break;
                case LogLevel_WARN: level_str = "[WARN]";
                    break;
                case LogLevel_ERROR: level_str = "[ERROR]";
                    break;
                case LogLevel_FATAL: level_str = "[FATAL]";
                    break;
                default: level_str = "[INFO]";
                    break;
            }

            return std::format(
                "{:%Y-%m-%d %H:%M:%S} [{}] {} {}",
                std::chrono::time_point_cast<std::chrono::milliseconds>(tp),
                *pack.name,
                level_str,
                f_msg
            );
        }
    };
}


#endif // ZCLIBLOG_FORMATTERS_FORMAT_HPP
