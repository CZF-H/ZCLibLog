// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_ANDROID_LOG_HPP
#define ZCLIBLOG_EXECUTORS_ANDROID_LOG_HPP

#include <android/log.h>

#include "../inside/logger_constants.hpp"

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        inline executor android_log(const char* tag) {
            return [tag](ELString msg, ELogLevel lv) {
                int android_level = ANDROID_LOG_INFO;
                switch (lv) {
                    case LogLevel_TRACE: android_level = ANDROID_LOG_VERBOSE; break;
                    case LogLevel_DEBUG: android_level = ANDROID_LOG_DEBUG;   break;
                    case LogLevel_INFO:  android_level = ANDROID_LOG_INFO;    break;
                    case LogLevel_WARN:  android_level = ANDROID_LOG_WARN;    break;
                    case LogLevel_ERROR: android_level = ANDROID_LOG_ERROR;   break;
                    case LogLevel_FATAL: android_level = ANDROID_LOG_FATAL;   break;
                    default: break;
                }
                __android_log_write(android_level, tag, msg.c_str());
            };
        }
        inline executor& android_log() {
            static executor inst = android_log(PROJECT_NAME);
            return inst;
        }
    }
}

#endif //ZCLIBLOG_EXECUTORS_ANDROID_LOG_HPP
