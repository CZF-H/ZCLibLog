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
        inline executor& iostream() {
            static executor inst = [](ELString msg, ELogLevel lv) {
                if (lv >= LogLevel_ERROR)
                    std::cerr << msg << std::endl;
                else if (lv >= LogLevel_INFO)
                    std::cout << msg << std::endl;
                else
                    std::clog << msg << std::endl;
            };
            return inst;
        }
    }
}

#endif //ZCLIBLOG_EXECUTORS_IOSTREAM_HPP
