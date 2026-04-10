#include <iostream>

#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/logger_async.hpp"

#include "ZCLibLog/formatters/format.hpp"

#include "ZCLibLog/executors/cstdout.hpp"

using LoggerType = ZCLibLog::LoggerSync<ZCLibLog::formatters::format>;
LoggerType Logger{
    ZCLibLog::PROJECT_NAME,
};


int main() {
    using namespace ZCLibLog;

    const auto Logger_executors_cstdout_id = Logger.bind_executor(executor::Construct<executors::cstdout>());

    const auto start = std::chrono::steady_clock::now();

    Logger.INFO("Hello {}!", PROJECT_NAME);

    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    Logger.debind_executor(Logger_executors_cstdout_id);

    std::cout << "Used: " << duration.count() << " ns" << std::endl;

    return 0;
}