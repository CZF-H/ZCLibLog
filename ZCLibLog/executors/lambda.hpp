// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/7.
//

#ifndef ZCLIBLOG_EXECUTORS_LAMBDA_HPP
#define ZCLIBLOG_EXECUTORS_LAMBDA_HPP

#include <functional>
#include <utility>
#include "../inside/logger_types.hpp"

namespace ZCLibLog {
    namespace executors {
        /** lambda thunk **/
        struct lambda : executor_api {
            using constructible = std::function<void(ELString, ELogLevel)>;
            explicit lambda(constructible constructed) : constructed(std::move(constructed)) {}
            void do_execute(ELString msg, ELogLevel lv) override {
                constructed(msg, lv);
            }
        private:
            constructible constructed;
        };
    }

    inline executor lambda_wrapper(executors::lambda::constructible constructed) {
        return executor::Construct<executors::lambda>(std::move(constructed));
    }
}

#endif //ZCLIBLOG_EXECUTORS_LAMBDA_HPP
