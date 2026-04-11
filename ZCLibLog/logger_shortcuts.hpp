// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_LOGGER_SHORTCUTS_HPP
#define ZCLIBLOG_LOGGER_SHORTCUTS_HPP

#include "logger_sync.hpp"
#include "logger_async.hpp"
#include "executors/cstdio.hpp"
#include "inside/logger_constants.hpp"

#define ZCLibLog_LOGALL(...) DefaultLoggerSync().ALL(__VA_ARGS__)
#define ZCLibLog_LOGTRACE(...) DefaultLoggerSync().TRACE(__VA_ARGS__)
#define ZCLibLog_LOGDEBUG(...) DefaultLoggerSync().DEBUG(__VA_ARGS__)
#define ZCLibLog_LOGINFO(...) DefaultLoggerSync().INFO(__VA_ARGS__)
#define ZCLibLog_LOGWARN(...) DefaultLoggerSync().WARN(__VA_ARGS__)
#define ZCLibLog_LOGERROR(...) DefaultLoggerSync().ERROR(__VA_ARGS__)
#define ZCLibLog_LOGFATAL(...) DefaultLoggerSync().FATAL(__VA_ARGS__)

#define ZCLibLog_ALOGALL(...) DefaultLoggerAsync().ALL(__VA_ARGS__)
#define ZCLibLog_ALOGTRACE(...) DefaultLoggerAsync().TRACE(__VA_ARGS__)
#define ZCLibLog_ALOGDEBUG(...) DefaultLoggerAsync().DEBUG(__VA_ARGS__)
#define ZCLibLog_ALOGINFO(...) DefaultLoggerAsync().INFO(__VA_ARGS__)
#define ZCLibLog_ALOGWARN(...) DefaultLoggerAsync().WARN(__VA_ARGS__)
#define ZCLibLog_ALOGERROR(...) DefaultLoggerAsync().ERROR(__VA_ARGS__)
#define ZCLibLog_ALOGFATAL(...) DefaultLoggerAsync().FATAL(__VA_ARGS__)

namespace ZCLibLog {
    namespace shortcuts  {
        inline LoggerSync<>& DefaultLoggerSync() {
            static LoggerSync<> logger{
                PROJECT_NAME,
                {
                    executor::make<executors::cstdio>()
                }
            };
            return logger;
        }
        inline LoggerAsync<>& DefaultLoggerAsync() {
            static LoggerAsync<> logger{
                PROJECT_NAME,
                {
                    executor::make<executors::cstdio>()
                }
            };
            return logger;
        }
    }
}

#endif //ZCLIBLOG_LOGGER_SHORTCUTS_HPP
