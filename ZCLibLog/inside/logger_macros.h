// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/4.
//

#ifndef ZCLIBLOG_LOGGER_MACROS_H
#define ZCLIBLOG_LOGGER_MACROS_H

#ifdef _MSC_VER
    #define ZCLibLog_CPP_VER _MSC_VER
#else
    #define ZCLibLog_CPP_VER __cplusplus
#endif

#if __cplusplus >= 202302L
    #define ZCLibLog_CPP_YEAR 2023
    #define ZCLibLog_CPP 23
#elif __cplusplus >= 202002L
    #define ZCLibLog_CPP_YEAR 2020
    #define ZCLibLog_CPP 20
#elif __cplusplus >= 201703L
    #define ZCLibLog_CPP_YEAR 2017
    #define ZCLibLog_CPP 17
#elif __cplusplus >= 201402L
    #define ZCLibLog_CPP_YEAR 2014
    #define ZCLibLog_CPP 14
#elif __cplusplus >= 201103L
    #define ZCLibLog_CPP_YEAR 2011
    #define ZCLibLog_CPP 11
#else
    #define ZCLibLog_CPP_YEAR 2003
    #define ZCLibLog_CPP 03
#endif

#if ZCLibLog_CPP >= 17
    #define ZCLibLog_NODISCARD [[nodiscard]]
#else
    #define ZCLibLog_NODISCARD
#endif

#if ZCLibLog_CPP >= 20
    #define ZCLibLog_HAS_FORMAT
    #define ZCLibLog_HAS_CONSTRAINTS
#endif

#endif //ZCLIBLOG_LOGGER_MACROS_H
