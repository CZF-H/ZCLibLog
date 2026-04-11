#include <iostream>

#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/logger_async.hpp"
#include "ZCLibLog/logger_shortcuts.hpp"

#include "ZCLibLog/formatters/snprintf.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

using LoggerType = ZCLibLog::LoggerSync<ZCLibLog::formatters::snprintf>;
LoggerType Logger{
    ZCLibLog::PROJECT_NAME,
};

int main() {
    using namespace ZCLibLog;

    const auto Logger_executors_cstdio_id = Logger.bind_executor(new executors::cstdio(true));

    const auto start = std::chrono::steady_clock::now();

    Logger.INFO("Hello %s!", PROJECT_NAME);

    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    Logger.debind_executor(Logger_executors_cstdio_id);

    std::cout << "Used: " << duration.count() << " ns" << std::endl;

    return 0;
}