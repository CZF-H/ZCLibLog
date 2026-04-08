# ZCLibLog

> 一个轻量、可扩展的 C++11 日志库：支持同步 / 异步 Logger、自定义 Formatter、可插拔 Executor。

![C++](https://img.shields.io/badge/language-C++-blue)
![C++](https://img.shields.io/badge/C++-C++11%2B-brightgreen)
![License](https://img.shields.io/github/license/CZF-H/ZCLibLog)

## ✨ 特点

- **双模型日志器**：`LoggerSync`（同步）与 `LoggerAsync`（异步线程池）。
- **可插拔 Formatter**：内置 `csnprintf`、`format`、`vformat`，也支持自定义。
- **可插拔 Executor**：内置终端、流、`FILE*`、Android Log、lambda 等输出执行器。
- **分级过滤**：`TRACE ~ FATAL`，可通过 `LogLevelCfg` 控制最小/最大等级。
- **头文件即用**：直接 `#include`，无需复杂运行时依赖。

---

## 📦 目录速览

- 日志器：
    - `ZCLibLog/logger_sync.hpp`
    - `ZCLibLog/logger_async.hpp`
- Formatter：
    - `ZCLibLog/formatters/csnprintf.hpp`
    - `ZCLibLog/formatters/format.hpp`
    - `ZCLibLog/formatters/vformat.hpp`
- Executor：
    - `ZCLibLog/executors/cstdout.hpp`
    - `ZCLibLog/executors/iostream.hpp`
    - `ZCLibLog/executors/cstdio.hpp`
    - `ZCLibLog/executors/ostream.hpp`
    - `ZCLibLog/executors/lambda.hpp`
    - `ZCLibLog/executors/android_log.hpp`

---

## 🔍 需要了解

### executor的构造方式

executor的构造方式目前有两种

1. 通过裸指针构造
2. 通过工厂函数`executor::Construct`构造

> 裸指针构造要确保对象生命周期比Logger更长

> 通过`new`构造的对象需要确保你不会`delete`
> 
> 🚫 如果对象不是在`executor`的构造函数中构造，请不要多次传入裸指针！
> 
> 📢 建议使用`executor::Construct`而不是裸指针构造
> 
> 📓 如果需要请遵循动态分配对象的立即传递

---

## 🚀 快速开始

### 1) 使用内置 Formatter + Executor 创建同步 Logger

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/formatters/format.hpp"      // C++20 std::format 风格
#include "ZCLibLog/executors/cstdout.hpp"      // 输出到 stdout/stderr

int main() {
    ZCLibLog::LoggerSync<ZCLibLog::formatters::format> logger{
        "MainLogger",
        {
            ZCLibLog::executor::Construct<ZCLibLog::executors::cstdout>()
        },
        ZCLibLog::LogLevel::INFO
    };

    logger.INFO("Hello {}", "ZCLibLog");
    logger.ERROR("Something bad happened: code={}", 500);
}
```

### 2) 使用内置 `csnprintf`（默认 Formatter）

> `LoggerSync<>` 在默认配置下会使用 `formatters::csnprintf`。

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/iostream.hpp"

int main() {
    ZCLibLog::LoggerSync<> logger{
        "DefaultFmt",
        {
            ZCLibLog::executor::Construct<ZCLibLog::executors::iostream>()
        },
        ZCLibLog::LogLevel::DEBUG
    };

    logger.DEBUG("id=%d, user=%s", 42, "alice");
}
```

### 3) 使用异步 Logger

```cpp
#include "ZCLibLog/logger_async.hpp"
#include "ZCLibLog/formatters/vformat.hpp"
#include "ZCLibLog/executors/cstdout.hpp"

int main() {
    ZCLibLog::LoggerAsync<ZCLibLog::formatters::vformat> logger{
        "AsyncLogger",
        {
            ZCLibLog::executor::Construct<ZCLibLog::executors::cstdout>()
        },
        ZCLibLog::LogLevel::TRACE
    };

    logger.TRACE("task={} started", 1001);
    logger.WARN("queue={} is almost full", "jobs");
}
```

---

## 🧩 内置组件说明

### Formatter

| 名称                        | 输入风格                     | 说明                       |
|---------------------------|--------------------------|--------------------------|
| `formatters::csnprintf`   | `printf` 样式              | 默认方案，兼容性好，性能稳定           |
| `formatters::format`      | `std::format_string`     | 类型安全，现代 C++20 风格         |
| `formatters::vformat`     | `std::string_view` + 参数包 | 便于动态格式串                  |
| `formatters::android_log` | `printf` 样式              | 面向 Android Logcat 的短格式输出 |

### Executor

| 名称                       | 输出目标                    | 说明                          |
|--------------------------|-------------------------|-----------------------------|
| `executors::cstdout`     | `stdout / stderr`       | `ERROR` 及以上输出到 `stderr`     |
| `executors::iostream`    | `std::cout / std::cerr` | C++ iostream 方式             |
| `executors::cstdio`      | `FILE*`                 | 写入 C 文件句柄并 `fflush`         |
| `executors::ostream`     | `std::ostream&`         | 任意输出流（如 `ofstream`）         |
| `executors::lambda`      | 自定义回调                   | 用 lambda 快速接入任意目标           |
| `executors::android_log` | Android Logcat          | 调用 `__android_log_write` 输出 |

---

## 🛠️ 教程：自写 Formatter

你的 Formatter 需要：

1. 继承 `ZCLibLog::format_apis::traditional` 或 `ZCLibLog::format_apis::stdcxx20`。
2. 提供静态函数 `do_format(FLogPack, ...) -> std::string`。

示例：

```cpp
#include "ZCLibLog/inside/logger_types.hpp"
#include <string>
#include <format>

struct MyFormatter : ZCLibLog::format_apis::stdcxx20 {
    template <typename... Args>
    static std::string do_format(
        FLogPack pack,
        std::format_string<Args...>&& fmt,
        Args&&... args
    ) {
        auto body = std::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
        return std::format("[{}][lv={}] {}", *pack.name, pack.level, body);
    }
};
```

接入：

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/cstdout.hpp"

ZCLibLog::LoggerSync<MyFormatter> logger{
    "CustomFmt",
    { ZCLibLog::executor::Construct<ZCLibLog::executors::cstdout>() },
    ZCLibLog::LogLevel::INFO
};
```

---

## 🔌 教程：自写 Executor

你的 Executor 需要：

1. 继承 `ZCLibLog::executor_api`。
2. 实现 `void do_execute(ELString msg, ELogLevel lv)`。

示例：

```cpp
#include "ZCLibLog/inside/logger_types.hpp"
#include <fstream>

struct FileExecutor : ZCLibLog::executor_api {
    explicit FileExecutor(const std::string& path) : out(path, std::ios::app) {}

    void do_execute(ELString msg, ELogLevel) override {
        if (out.is_open()) {
            out << msg << '\n';
            out.flush();
        }
    }

private:
    std::ofstream out;
};
```

接入：

```cpp
#include "ZCLibLog/logger_sync.hpp"

ZCLibLog::LoggerSync<> logger{
    "FileLogger",
    { ZCLibLog::executor::Construct<FileExecutor>("app.log") },
    ZCLibLog::LogLevel::DEBUG
};
```

---

## 🧠 进阶：动态绑定 / 解绑定执行器

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/cstdout.hpp"

ZCLibLog::LoggerSync<> logger{"Dynamic"};

size_t id = logger.bind_executor(ZCLibLog::executor::Construct<ZCLibLog::executors::cstdout>());
logger.INFO("executor attached");

logger.debind_executor(id);
logger.INFO("this line will not be printed");
```

---

## 🧪 进阶：lambda 包装器与 Android 用法

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/lambda.hpp"

std::vector<std::string> sink;

ZCLibLog::LoggerSync<> logger{"LambdaSink"};
auto ex = ZCLibLog::lambda_wrapper([&](ZCLibLog::ELString msg, ZCLibLog::ELogLevel) {
    sink.push_back(msg);
});

logger.bind_executor(ex);
logger.INFO("captured to vector");
```

Android 平台可组合 `formatters::android_log + executors::android_log`：

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/formatters/android_log.hpp"
#include "ZCLibLog/executors/android_log.hpp"

ZCLibLog::LoggerSync<ZCLibLog::formatters::android_log> logger{
    "AndroidLogger",
    { ZCLibLog::executor::Construct<ZCLibLog::executors::android_log>("MyApp") },
    ZCLibLog::LogLevel::INFO
};
```

---

## 🛡️ 保护与最佳实践

1. **执行器生命周期管理**：
    - 推荐使用 `executor::Construct`辅助函数来管理资源，避免`double delete`。
2. **外部资源有效性保护**：
    - `cstdio(FILE*&)`、`ostream(std::ostream&)` 依赖外部引用，请确保日志期间对象仍然有效。
3. **级别过滤保护**：
    - 使用 `LogLevelCfg(min, max)` 约束输出范围，避免大量低级别日志冲击性能。
4. **异步使用建议**：
    - 高频日志下优先 `LoggerAsync`，并根据业务调整线程数（`logger_configurations.h`）。
5. **格式串匹配保护**：
    - `csnprintf` 需严格保证格式串与参数匹配。
    - `format` / `vformat` 在类型安全上更友好（推荐 C++20 环境优先使用）。
6. **Android 构建保护**：
    - 仅在 Android NDK 环境下包含 `android_log` 相关头文件，避免非 Android 平台编译失败。

---

## ⚙️ 构建示例

本仓库已提供一个示例程序 `main.cpp`：

```bash
cmake -S . -B build
cmake --build build -j
./build/ZCLibLog_Example
```

---

## 📄 许可证

Apache-2.0

---

## 📢 声明

此文档使用ChatGPT Codex生成，如内容有误请您谅解，感谢指出错误，谢谢
