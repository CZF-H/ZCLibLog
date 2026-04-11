# ZCLibLog

> 轻量、可扩展的 C++ 日志库（C++11+），支持同步/异步 Logger、可插拔 Formatter、可插拔 Executor。

![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![Standard](https://img.shields.io/badge/C%2B%2B-11%2B-brightgreen)
![License](https://img.shields.io/github/license/CZF-H/ZCLibLog)

---

### **简体中文** | [English](./README.en.md) 

---

## 📚 文档

- Doxygen: https://czf-h.github.io/ZCLibLog

---

## 1. 项目定位

ZCLibLog 是头文件风格日志库，核心设计是：

- `Logger` 负责路由、过滤、调度。
- `Formatter` 负责把业务参数格式化成字符串。
- `Executor` 负责把最终字符串输出到具体目标（终端、文件、网络、回调等）。

三者解耦，因此你可以自由组合，也可以只替换其中一层。

---

## 2. 快速开始

### 2.1 同步 Logger + 默认 Formatter（`csnprintf`）

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

int main() {
    ZCLibLog::LoggerSync<> logger{"SyncDefault"};
    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::cstdio>(true));

    logger.INFO("Hello %s", "ZCLibLog");
    logger.ERROR("code=%d", 500);
}
```

### 2.2 同步 Logger + C++20 `std::format`

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/formatters/format.hpp"
#include "ZCLibLog/executors/iostream.hpp"

int main() {
    ZCLibLog::LoggerSync<ZCLibLog::formatters::format> logger{"FmtLogger"};
    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::iostream>());

    logger.DEBUG("id={} user={}", 42, "alice");
}
```

### 2.3 异步 Logger

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

---

## 3. 内置组件总览

### 3.1 Formatter

| 名称                        | 风格                        | 说明                  |
|---------------------------|---------------------------|---------------------|
| `formatters::csnprintf`   | `printf`                  | 默认方案，兼容 C++11，跨平台稳妥 |
| `formatters::format`      | `std::format_string`      | C++20 编译期检查风格       |
| `formatters::vformat`     | `std::string_view + args` | C++20 动态 format 字符串 |
| `formatters::android_log` | Android 风格                | 面向 Logcat           |

### 3.2 Executor

| 名称                       | 输出目标                  |
|--------------------------|-----------------------|
| `executors::cstdio`      | `stdout/stderr`       |
| `executors::cfwrite`     | `FILE*`（`fwrite`）     |
| `executors::cfputs`      | `FILE*`（`fputs`）      |
| `executors::cputs`       | C 标准输出                |
| `executors::iostream`    | `std::cout/std::cerr` |
| `executors::ostream`     | 任意 `std::ostream&`    |
| `executors::print`       | 标准输出封装                |
| `executors::lambda`      | Lambda 回调桥接           |
| `executors::android_log` | Android Logcat        |

---

## 4. 自定义 Formatter（重点）

### 4.1 接口规则

要自定义 Formatter，本质上你需要：

1. 继承某个 `format_api`（通常是 `format_apis::traditional` 或 `format_apis::stdcxx20format`）。
2. 提供 `static std::string do_format(...)`。
3. 第一个参数是 `FLogPack pack`，后面是你的格式化参数签名。

### 4.2 示例：自定义 JSON Formatter（traditional）

```cpp
#include "ZCLibLog/formatters/format_apis/traditional.hpp"
#include "ZCLibLog/inside/logger_types.hpp"
#include <cstdio>
#include <string>

namespace MyLog {
    struct JsonFormatter : ZCLibLog::format_apis::traditional {
        template<typename... Args>
        static std::string do_format(FLogPack pack, const char* fmt, Args&&... args) {
            char msg[1024]{};
            std::snprintf(msg, sizeof(msg), fmt, std::forward<Args>(args)...);

            char out[1400]{};
            std::snprintf(
                out,
                sizeof(out),
                "{\"name\":\"%s\",\"level\":\"%s\",\"time\":%llu,\"msg\":\"%s\"}",
                pack.name ? pack.name->c_str() : "",
                ZCLibLog::LogLevelToString(pack.level),
                static_cast<unsigned long long>(pack.time),
                msg
            );
            return out;
        }
    };
}
```

使用方式：

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

int main() {
    ZCLibLog::LoggerSync<MyLog::JsonFormatter> logger{"JsonLogger"};
    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::cstdio>());

    logger.INFO("uid=%d action=%s", 7, "login");
}
```

### 4.3 示例：C++20 严格类型 Formatter（`stdcxx20format`）

```cpp
#include "ZCLibLog/formatters/format_apis/stdcxx20format.hpp"
#include <format>

namespace MyLog {
    struct StrictFmt : ZCLibLog::format_apis::stdcxx20format {
        template <typename... Args>
        static std::string do_format(FLogPack pack, std::format_string<Args...>&& fmt, Args&&... args) {
            auto body = std::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
            return std::format("[{}][{}] {}", *pack.name, ZCLibLog::LogLevelToString(pack.level), body);
        }
    };
}
```

---

## 5. 自定义 Executor（重点）

### 5.1 接口规则

自定义 Executor 需要：

1. 继承 `executor_api`。
2. 重写 `void do_execute(ELString msg, ELogLevel lv)`。

### 5.2 示例：按级别输出到不同文件

```cpp
#include "ZCLibLog/executors/basic_executor.hpp"
#include <fstream>

namespace MyLog {
    struct SplitFileExecutor : ZCLibLog::executor_api {
        std::ofstream normal{"app.log", std::ios::app};
        std::ofstream error{"app.err.log", std::ios::app};

        void do_execute(ELString msg, ELogLevel lv) override {
            if (lv >= LogLevel::ERROR) error << msg << '\n';
            else normal << msg << '\n';
        }
    };
}
```

绑定：

```cpp
auto ex = ZCLibLog::executor::make<MyLog::SplitFileExecutor>();
logger.bind_executor(ex);
```

### 5.3 示例：Windows `WARN/ERROR/FATAL` 自动声音提示（进阶玩法）

```cpp
#include "ZCLibLog/executors/basic_executor.hpp"
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace MyLog {
    struct BeepExecutor : ZCLibLog::executor_api {
        void do_execute(ELString msg, ELogLevel lv) override {
            std::cout << msg << std::endl;

            if (lv >= LogLevel::WARN) {
#if defined(_WIN32)
                // Windows 系统提示音（推荐）
                MessageBeep(MB_ICONWARNING);
                // 或者使用 Beep(1200, 80);
#else
                // 非 Windows 退化为控制台响铃字符
                std::cout << "\a" << std::flush;
#endif
            }
        }
    };
}
```

> 建议：声音提示 Executor 与正常输出 Executor 分开绑定，便于按场景启停。

### 5.4 示例：网络上报 Executor（伪代码框架）

```cpp
struct HttpExecutor : ZCLibLog::executor_api {
    void do_execute(ELString msg, ELogLevel lv) override {
        // enqueue(msg, lv) -> worker thread -> HTTP POST
        // 生产环境建议：批量、重试、退避、熔断
    }
};
```

---

## 6. 自定义 Logger（继承 `BaseLogger`）

当你需要在 Logger 层做统一策略（例如：
- 额外上下文注入，
- 多播路由，
- 自定义执行策略），
可直接继承 `BaseLogger<Formatter>`。

### 6.1 最小可用自定义 Logger

```cpp
#include "ZCLibLog/logger_base.hpp"
#include "ZCLibLog/formatters/csnprintf.hpp"

namespace MyLog {
    template <typename Formatter = ZCLibLog::formatters::csnprintf>
    struct MyLogger : ZCLibLog::BaseLogger<Formatter> {
        using Base = ZCLibLog::BaseLogger<Formatter>;
        using Base::Base;
        using Self = MyLogger<Formatter>;

        void execute(const std::string& message, const ZCLibLog::LogLevel level) const {
            if (message.empty()) return;
            for (const auto& ex_pair : this->m_executors) {
                ex_pair.second->do_execute(message, level);
            }
        }

        using Tag = ZCLibLog::LogTag<typename Formatter::Format_API, Self>;

        Tag TRACE{this, ZCLibLog::LogLevel::TRACE};
        Tag DEBUG{this, ZCLibLog::LogLevel::DEBUG};
        Tag INFO{this, ZCLibLog::LogLevel::INFO};
        Tag WARN{this, ZCLibLog::LogLevel::WARN};
        Tag ERROR{this, ZCLibLog::LogLevel::ERROR};
        Tag FATAL{this, ZCLibLog::LogLevel::FATAL};
    };
}
```

### 6.2 扩展例：覆写 execute 做“多策略派发”

```cpp
void execute(const std::string& message, const ZCLibLog::LogLevel level) const {
    if (message.empty()) return;

    // 例：仅 ERROR+ 才全量执行器，其他等级只发前 1 个执行器
    if (level >= ZCLibLog::LogLevel::ERROR) {
        for (const auto& ex_pair : this->m_executors) ex_pair.second->do_execute(message, level);
    } else if (!this->m_executors.empty()) {
        this->m_executors.front().second->do_execute(message, level);
    }
}
```

---

## 7. 推荐实践（非常重要）

1. **优先 `executor::make<T>(...)`，谨慎裸指针。**  
   裸指针交给 `executor` 后不要手动 delete。
2. **高频路径避免重分配。**  
   Formatter 内可复用 `thread_local` buffer（内置 `csnprintf` 已示例）。
3. **把“慢 I/O”下沉到异步。**  
   网络、磁盘大量写入建议用 `LoggerAsync` + 队列化 Executor。
4. **日志格式尽量结构化。**  
   推荐 JSON 或 KV，便于后续 ELK / Loki / ClickHouse 检索。
5. **按级别做资源分流。**  
   `INFO` 走普通通道，`ERROR+` 走独立告警通道（短信/IM/声响）。
6. **控制日志风暴。**  
   可在 Executor 层做限速、去重、采样。

---

## 8. 进阶玩法清单

- **玩法 A：双 Formatter 并存**  
  普通日志走文本，审计日志走 JSON（通过两个 Logger 实例实现）。
- **玩法 B：A/B Executor**  
  同时绑定 console + file + http，实现实时看日志和后端归档。
- **玩法 C：全局短宏快速落地**  
  使用 `logger_shortcuts.hpp` 的默认 logger 快速接入。
- **玩法 D：Windows 音频告警**  
  见 `BeepExecutor` 示例，对 `WARN/ERROR/FATAL` 触发音效。
- **玩法 E：按模块建立 logger**  
  如 `AuthLogger` / `OrderLogger` / `DBLogger`，隔离噪音。

---

## 9. 常见问题（FAQ）

### Q1：为什么 `LoggerSync<>` 不传 Formatter 也能用？
因为默认配置下启用了 `csnprintf`（见 `logger_configurations.h`）。

### Q2：我应该用 `format` 还是 `vformat`？
- 固定格式字符串、追求类型安全：`format`
- 运行时动态模板：`vformat`
- 最大兼容（C++11）：`csnprintf`

### Q3：自定义 Formatter 报错“must be format_api”？
说明你的 Formatter 没有继承任何 `format_api` 类型。

### Q4：我能否扩展日志等级？
可以，在 `inside/logger_types.hpp` 的等级宏处可扩展（注意与现有比较逻辑一致）。

---

## 10. 构建与运行

```bash
cmake -S . -B build
cmake --build build -j
./build/ZCLibLog_Example
```

---

## 📢 声明

此文档使用ChatGPT Codex生成，如内容有误请您谅解，感谢指出错误，谢谢

---

## 📄 许可证

### [Apache-2.0](LICENSE)