# ZCLibLog

> 轻量、可扩展的 C++ 日志库（C++11+），支持同步 / 异步 Logger、可插拔 Formatter、可插拔 Executor。

![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![Standard](https://img.shields.io/badge/C%2B%2B-11%2B-brightgreen)
![License](https://img.shields.io/github/license/CZF-H/ZCLibLog)

## ✨ 特性

- 同时支持 `LoggerSync`（同步）与 `LoggerAsync`（异步线程池）。
- Formatter 与 Executor 解耦，可按需替换 / 扩展。
- 提供 `TRACE ~ FATAL` 级别与范围过滤（`LogLevelCfg`）。
- 头文件库，接入成本低。

## 📚 文档

- Doxygen: https://czf-h.github.io/ZCLibLog/index.html

## 📦 目录速览

- 日志器
  - `ZCLibLog/logger_sync.hpp`
  - `ZCLibLog/logger_async.hpp`
- Formatter
  - `ZCLibLog/formatters/csnprintf.hpp`
  - `ZCLibLog/formatters/format.hpp`
  - `ZCLibLog/formatters/vformat.hpp`
  - `ZCLibLog/formatters/android_log.hpp`
- Executor
  - `ZCLibLog/executors/cstdio.hpp`
  - `ZCLibLog/executors/cfwrite.hpp`
  - `ZCLibLog/executors/cfputs.hpp`
  - `ZCLibLog/executors/cputs.hpp`
  - `ZCLibLog/executors/iostream.hpp`
  - `ZCLibLog/executors/ostream.hpp`
  - `ZCLibLog/executors/print.hpp`
  - `ZCLibLog/executors/lambda.hpp`
  - `ZCLibLog/executors/android_log.hpp`

## 🚀 快速开始

### 1) 同步 Logger（`formatters::format` + `executors::cstdio`）

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/formatters/format.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

int main() {
    ZCLibLog::LoggerSync<ZCLibLog::formatters::format> logger{"MainLogger"};

    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::cstdio>(true));

    logger.INFO("Hello {}", "ZCLibLog");
    logger.ERROR("code={}", 500);
}
```

### 2) 默认 Formatter（`csnprintf`）

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/iostream.hpp"

int main() {
    ZCLibLog::LoggerSync<> logger{"DefaultFmt"};

    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::iostream>(true));
    logger.DEBUG("id=%d user=%s", 42, "alice");
}
```

### 3) 异步 Logger

```cpp
#include "ZCLibLog/logger_async.hpp"
#include "ZCLibLog/formatters/vformat.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

int main() {
    ZCLibLog::LoggerAsync<ZCLibLog::formatters::vformat> logger{"AsyncLogger"};

    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::cstdio>(true));
    logger.TRACE("task={} started", 1001);
}
```

## 🔍 executor 构造建议

目前支持两种方式：

1. 直接传入裸指针。
2. 使用 `executor::make` 工厂函数。

建议优先使用 `executor::make`，减少对象生命周期与重复释放风险。

## 🧩 内置组件

### Formatter

| 名称 | 风格 | 说明 |
|---|---|---|
| `formatters::csnprintf` | `printf` | 默认方案，兼容性高 |
| `formatters::format` | `std::format_string` | C++20 类型安全风格 |
| `formatters::vformat` | `std::string_view + args` | 便于动态格式化 |
| `formatters::android_log` | `printf` | Android Logcat 友好输出 |

### Executor

| 名称 | 输出目标 |
|---|---|
| `executors::cstdio` | `stdout / stderr` |
| `executors::cfwrite` | `FILE*`（`fwrite`） |
| `executors::cfputs` | `FILE*`（`fputs`） |
| `executors::cputs` | C 标准输出 |
| `executors::iostream` | `std::cout / std::cerr` |
| `executors::ostream` | `std::ostream&` |
| `executors::print` | 标准输出封装 |
| `executors::lambda` | 自定义回调 |
| `executors::android_log` | Android Logcat |

## ⚙️ 构建示例

仓库内已提供示例程序 `main.cpp`：

```bash
cmake -S . -B build
cmake --build build -j
./build/ZCLibLog_Example
```

## 📄 许可证

Apache-2.0
