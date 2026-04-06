// Copyright 2026 CZF-H
// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/4.
//

#ifndef ZCLIBLOG_LOGGER_CONFIGURATIONS_H
#define ZCLIBLOG_LOGGER_CONFIGURATIONS_H

// 是否为Logger添加默认csnprintf formatter
#define ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF true

// 异步Logger默认线程数量
#define ZCLIBLOG_LOGGER_CONFIGURATIONS_ASYNC_THREAD_NUM 1

// 启用C++20字面量format (不影响vformat)
#define ZCLIBLOG_LOGGER_CONFIGURATIONS_ENABLE_CXX20_FORMAT true

// LoggerSync资源加锁
#define ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_SYNC_MUTEX true

// LoggerAsync资源加锁
#define ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX true

#endif //ZCLIBLOG_LOGGER_CONFIGURATIONS_H
