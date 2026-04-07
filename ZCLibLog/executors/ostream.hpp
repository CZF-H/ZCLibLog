// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_OSTREAM_HPP
#define ZCLIBLOG_EXECUTORS_OSTREAM_HPP

#include <ostream>
#include "../inside/logger_types.hpp"

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @warning 请确保执行时输出流没有被销毁
         */
        struct ostream : executor_api {
            explicit ostream(std::ostream& os) : os(os) {}
            void do_execute(ELString msg, ELogLevel) override {
                os << msg << std::endl;
            }
        private:
            std::ostream& os;
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_OSTREAM_HPP
