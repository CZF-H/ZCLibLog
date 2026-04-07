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
         * @warning 请确保 FILE* 在执行器使用期间有效
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
