// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_EXECUTORS_CSTDOUT_HPP
#define ZCLIBLOG_EXECUTORS_CSTDOUT_HPP

#include <cstdio>
#include "../inside/logger_types.hpp"

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        struct cstdout : executor_api {
            void do_execute(ELString msg, ELogLevel lv) override {
                if (lv >= LogLevel::ERROR) {
                    fputs(msg.c_str(), stderr);
                    fputs("\n", stderr);
                } else {
                    fputs(msg.c_str(), stdout);
                    fputs("\n", stdout);
                }
            }
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_CSTDOUT_HPP
