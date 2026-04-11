# ZCLibLog English Documentation (Full Advanced Guide)

**English** | [中文](./README.zh-CN.md)

> This guide covers the full path from quick integration to deep customization:
> 1) Custom Formatter  
> 2) Custom Executor  
> 3) Custom Logger (inherit from `BaseLogger`)  
> 4) Practical advice, extension ideas, and advanced patterns (including Windows beep on `WARN/ERROR/FATAL`)

---

## 1. What ZCLibLog is

ZCLibLog is a header-only style logging library with 3 decoupled layers:

- `Logger`: filtering, routing, dispatching
- `Formatter`: converting payload to final string
- `Executor`: sending final string to output target

You can replace any layer independently.

---

## 2. Quick start

### 2.1 Sync Logger + default formatter (`csnprintf`)

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

### 2.2 Sync Logger + C++20 `std::format`

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

### 2.3 Async Logger

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

## 3. Built-in components

### 3.1 Formatter

| Name | Style | Notes |
|---|---|---|
| `formatters::csnprintf` | `printf` | default, C++11-friendly |
| `formatters::format` | `std::format_string` | C++20 compile-time checked format |
| `formatters::vformat` | `std::string_view + args` | C++20 runtime format string |
| `formatters::android_log` | Android style | Logcat-oriented |

### 3.2 Executor

| Name | Output |
|---|---|
| `executors::cstdio` | `stdout/stderr` |
| `executors::cfwrite` | `FILE*` (`fwrite`) |
| `executors::cfputs` | `FILE*` (`fputs`) |
| `executors::cputs` | C output |
| `executors::iostream` | `std::cout/std::cerr` |
| `executors::ostream` | any `std::ostream&` |
| `executors::print` | print wrapper |
| `executors::lambda` | lambda callback bridge |
| `executors::android_log` | Android Logcat |

---

## 4. Custom Formatter (core extension)

### 4.1 Contract

A custom Formatter should:

1. inherit from a `format_api` (usually `format_apis::traditional` or `format_apis::stdcxx20format`),
2. provide `static std::string do_format(...)`,
3. accept `FLogPack pack` as first argument.

### 4.2 Example: JSON formatter (`traditional`)

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

Use it:

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

int main() {
    ZCLibLog::LoggerSync<MyLog::JsonFormatter> logger{"JsonLogger"};
    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::cstdio>());

    logger.INFO("uid=%d action=%s", 7, "login");
}
```

---

## 5. Custom Executor (core extension)

### 5.1 Contract

A custom Executor should:

1. inherit from `executor_api`,
2. override `void do_execute(ELString msg, ELogLevel lv)`.

### 5.2 Example: split by level into different files

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

Bind:

```cpp
auto ex = ZCLibLog::executor::make<MyLog::SplitFileExecutor>();
logger.bind_executor(ex);
```

### 5.3 Example: Windows beep on `WARN/ERROR/FATAL`

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
                MessageBeep(MB_ICONWARNING);  // recommended
#else
                std::cout << "\a" << std::flush;
#endif
            }
        }
    };
}
```

---

## 6. Custom Logger (inherit `BaseLogger`)

If you need logger-level strategies (context injection, custom routing, selective dispatch), inherit from `BaseLogger<Formatter>`.

### 6.1 Minimal custom logger

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

---

## 7. Recommendations

1. Prefer `executor::make<T>(...)` over raw pointers.
2. Reduce allocations on hot path (`thread_local` reusable buffers).
3. Move slow I/O to async path (`LoggerAsync` + buffered executors).
4. Prefer structured logs (JSON/KV) for observability stack.
5. Split channels by severity (`INFO` vs `ERROR+` alerts).
6. Add rate limiting / dedup / sampling in executor layer.

---

## 8. Advanced patterns

- Dual loggers: plain text + JSON audit logger.
- Multi-executor fanout: console + file + remote sink.
- Global quick macros via `logger_shortcuts.hpp`.
- Windows audible alerts for critical levels.
- Module-specific loggers (`Auth`, `Order`, `DB`).

---

## 9. Build

```bash
cmake -S . -B build
cmake --build build -j
./build/ZCLibLog_Example
```

---

## 10. Fast multi-version switch

- Entry: `README.md`
- Chinese: `README.zh-CN.md`
- English: `README.en.md`

