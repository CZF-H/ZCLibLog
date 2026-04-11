# ZCLibLog

> Lightweight and extensible C++ logging library (C++11+), with sync/async loggers, pluggable formatter, and pluggable executor.
>
> 轻量、可扩展的 C++ 日志库（C++11+），支持同步/异步 Logger、可插拔 Formatter、可插拔 Executor。

![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![Standard](https://img.shields.io/badge/C%2B%2B-11%2B-brightgreen)
![License](https://img.shields.io/github/license/CZF-H/ZCLibLog)

## 🌐 Language / 语言切换

- [中文文档（详细）](./README.zh-CN.md)
- [English Docs (Detailed)](./README.en.md)

## 📚 Documentation

- Doxygen: https://czf-h.github.io/ZCLibLog/index.html

## 🚀 Quick Start

### CMake

```bash
cmake -S . -B build
cmake --build build -j
./build/ZCLibLog_Example
```

### Minimal sample

```cpp
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/executors/cstdio.hpp"

int main() {
    ZCLibLog::LoggerSync<> logger{"MainLogger"};
    logger.bind_executor(ZCLibLog::executor::make<ZCLibLog::executors::cstdio>(true));
    logger.INFO("hello %s", "world");
    return 0;
}
```

## 🔁 Fast switch tips

- Chinese first: open `README.zh-CN.md`
- English first: open `README.en.md`
- Keep both files side-by-side in IDE split view for quick cross-reference.

## 📄 License

Apache-2.0
