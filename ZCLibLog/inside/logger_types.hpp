//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_CLASSES_HPP
#define ZCLIBLOG_LOGGER_CLASSES_HPP

#include <string>
#include <thread>
#include <functional>

namespace ZCLibLog {
    using LogLevel = uint16_t;

    enum LogLevel_ : LogLevel {
        LogLevel_ALL = 0,

        LogLevel_TRACE = 1,
        LogLevel_DEBUG = 2,
        LogLevel_INFO  = 3,
        LogLevel_WARN  = 4,
        LogLevel_ERROR = 5,
        LogLevel_FATAL = 6,

        LogLevel_NONE = std::numeric_limits<LogLevel>::max()
    };

    struct LogPack {
        const std::string* module = {};
        uint64_t time = {};
        LogLevel level = {};
    };

    using ELogLevel = const LogLevel;    // 执行器接收的等级
    using ELString = const std::string&; // 执行器接受的字符串

    using executor = std::function<void(ELString, ELogLevel)>;

    using FLogPack = const LogPack&;    // 格式化接受的数据包

    struct cxx20_format_api;
    struct traditional_fmt_api;
}

#endif //ZCLIBLOG_LOGGER_CLASSES_HPP
