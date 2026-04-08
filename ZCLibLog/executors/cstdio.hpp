// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by TingIAAI on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_CSTDIO_HPP
#define ZCLIBLOG_EXECUTORS_CSTDIO_HPP

#include <cstdio>
#include "../inside/logger_types.hpp"

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct cstdio
         * @brief C语言"stdio"通用执行器
         * @warning 请确保执行器存在时传入的FILE*没有被销毁
         * @author TingIAAI
         */
        struct cstdio : executor_api {
            explicit cstdio(FILE*& f) : f(f) {}
            void do_execute(ELString msg, ELogLevel) override {
                if (f) {
                    fputs(msg.c_str(), f);
                    fputs("\n", f);
                    fflush(f);
                }
            }
        private:
            FILE*& f;
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_CSTDIO_HPP
