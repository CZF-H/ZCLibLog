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
        inline executor cstdio(FILE* f) {
            return [f](ELString msg, ELogLevel lv) {
                if (f) {
                    fputs(msg.c_str(), f);
                    fputs("\n", f);
                    fflush(f);
                }
            };
        }
    }
}

#endif //ZCLIBLOG_EXECUTORS_CSTDIO_HPP
