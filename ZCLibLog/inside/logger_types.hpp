//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_CLASSES_HPP
#define ZCLIBLOG_LOGGER_CLASSES_HPP

#include <string>
#include <thread>
#include <memory>
#include <type_traits>

namespace ZCLibLog {
    using LogLevel = uint16_t;

    enum LogLevel_ : LogLevel {
        LogLevel_ALL = LogLevel{},

        LogLevel_TRACE = 1,
        LogLevel_DEBUG = 2,
        LogLevel_INFO  = 3,
        LogLevel_WARN  = 4,
        LogLevel_ERROR = 5,
        LogLevel_FATAL = 6,

        LogLevel_NONE = std::numeric_limits<LogLevel>::max()
    };

    struct LogPack {
        const std::string* name = {};
        uint64_t time = {};
        LogLevel level = {};
    };

    struct LogConfig {
        LogLevel min_level;
        LogLevel max_level;

        // ReSharper disable once CppNonExplicitConvertingConstructor
        LogConfig(const LogLevel level = {}) : min_level(level), max_level(LogLevel_NONE) {}
        LogConfig(const LogLevel min_level, const LogLevel max_level) : min_level(min_level), max_level(max_level) {}
    };

    using ELogLevel = const LogLevel;    // 执行器接收的等级
    using ELString = const std::string&; // 执行器接受的字符串

    struct executor_api {
        virtual void do_execute(ELString, ELogLevel) = 0;
        virtual ~executor_api() = default;
    };

    template <typename Executor>
    using is_executor_api = std::is_base_of<executor_api, Executor>;

    class executor {
    public:
        using base_ptr_type = std::shared_ptr<executor_api>;
        using inside_type = base_ptr_type::element_type;
    private:
        base_ptr_type executor_ptr;
    public:
        executor() = default;
        // ReSharper disable once CppNonExplicitConvertingConstructor
        /**
         * @warning 裸指针传入后禁止delete!
         */
        executor(inside_type* executor_ptr) : executor_ptr(executor_ptr) {}

        template<typename Executor, typename... Args>
        static executor Construct(Args&&... args) {
            static_assert(is_executor_api<Executor>::value, "Executor must be a real executor");

            executor Constructed;
            Constructed.executor_ptr = std::move(std::make_shared<Executor>(std::forward<Args>(args)...));

            return std::move(Constructed);
        }

        ZCLibLog_NODISCARD inside_type* get() const noexcept {
            return executor_ptr.get();
        }

        ZCLibLog_NODISCARD bool unique() const noexcept {
            // return executor_ptr.unique();
            return executor_ptr.use_count() == 1;
        }

        ZCLibLog_NODISCARD long use_count() const noexcept {
            return executor_ptr.use_count();
        }

        inside_type* operator->() const noexcept {
            return executor_ptr.operator->();
        }

        inside_type& operator*() const noexcept {
            return executor_ptr.operator*();
        }

        // ReSharper disable once CppNonExplicitConversionOperator
        operator bool() const noexcept {
            return executor_ptr.get() != nullptr;
        }
    };

    using FLogPack = const LogPack&;    // 格式化接受的数据包

    struct format_api{};
    namespace format_apis {
        struct stdcxx20 : format_api {};
        struct traditional : format_api{};
    };

    template <typename Formatter, typename FormatAPI = format_api>
    using is_format_api = std::is_base_of<FormatAPI, Formatter>;
}

#endif //ZCLIBLOG_LOGGER_CLASSES_HPP
