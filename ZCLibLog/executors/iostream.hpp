// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_EXECUTORS_IOSTREAM_HPP
#define ZCLIBLOG_EXECUTORS_IOSTREAM_HPP

#include <iostream>
#include "../inside/logger_types.hpp"

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        struct iostream : executor_api {
            void do_execute(ELString msg, ELogLevel lv) override {
                if (lv >= LogLevel::ERROR)
                    std::cerr << msg << std::endl;
                else
                    std::cout << msg << std::endl;
            }
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_IOSTREAM_HPP
