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
        inline executor& cstdout() {
            static executor inst = [](ELString msg, ELogLevel lv) {
                if (lv >= LogLevel_ERROR) {
                    fputs(msg.c_str(), stderr);
                    fputs("\n", stderr);
                } else {
                    fputs(msg.c_str(), stdout);
                    fputs("\n", stdout);
                }
            };
            return inst;
        }
    }
}

#endif //ZCLIBLOG_EXECUTORS_CSTDOUT_HPP
