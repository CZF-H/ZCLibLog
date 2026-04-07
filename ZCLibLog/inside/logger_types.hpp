//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_CLASSES_HPP
#define ZCLIBLOG_LOGGER_CLASSES_HPP

#include <string>
#include <thread>
#include <functional>
#include <type_traits>

namespace ZCLibLog {
    using LogLevel = uint16_t;

    enum LogLevel_ : LogLevel {
        LogLevel_ALL = LogLevel{},

        LogLevel_TRACE = 1,
        LogLevel_DEBUG = 2,
        LogLevel_INFO  = 3,
        LogLevel_WARN  = 4,
        LogLevel_ERROR = 5,
        LogLevel_FATAL = 6,

        LogLevel_NONE = std::numeric_limits<LogLevel>::max()
    };

    struct LogPack {
        const std::string* name = {};
        uint64_t time = {};
        LogLevel level = {};
    };

    struct LogConfig {
        LogLevel min_level;
        LogLevel max_level;

        // ReSharper disable once CppNonExplicitConvertingConstructor
        LogConfig(const LogLevel level = {}) : min_level(level), max_level(LogLevel_NONE) {}
        LogConfig(const LogLevel min_level, const LogLevel max_level) : min_level(min_level), max_level(max_level) {}
    };

    using ELogLevel = const LogLevel;    // 执行器接收的等级
    using ELString = const std::string&; // 执行器接受的字符串

    using executor = std::function<void(ELString, ELogLevel)>;

    using FLogPack = const LogPack&;    // 格式化接受的数据包

    struct format_api{};
    struct format_apis {
        struct stdcxx20 : format_api {};
        struct traditional : format_api{};
    };

    template <typename Formatter, typename FormatAPI = format_api>
    using is_format_api = std::is_base_of<FormatAPI, Formatter>;
}

#endif //ZCLIBLOG_LOGGER_CLASSES_HPP
