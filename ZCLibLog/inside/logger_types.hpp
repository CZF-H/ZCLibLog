//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_TYPES_HPP
#define ZCLIBLOG_LOGGER_TYPES_HPP

#include <string>
#include <thread>
#include <memory>
#include <type_traits>

namespace ZCLibLog {
    using LogLevelBase = uint16_t;

    enum class LogLevel : LogLevelBase {
        ALL = LogLevelBase{},

        TRACE = 1,
        DEBUG = 2,
        INFO  = 3,
        WARN  = 4,
        ERROR = 5,
        FATAL = 6,

        NONE = std::numeric_limits<LogLevelBase>::max()
    };

    struct LogPack {
        const std::string* name = {};
        uint64_t time = {};
        LogLevel level = {};
    };

    struct LogLevelCfg {
        LogLevel min_level;
        LogLevel max_level;

        // ReSharper disable once CppNonExplicitConvertingConstructor
        LogLevelCfg(const LogLevel level = {}) : min_level(level), max_level(LogLevel::NONE) {}
        LogLevelCfg(const LogLevel min_level, const LogLevel max_level) : min_level(min_level), max_level(max_level) {}
    };

    using ELogLevel = const LogLevel;    // 执行器接收的等级
    using ELString = const std::string&; // 执行器接受的字符串

    struct executor_api {
        virtual void do_execute(ELString, ELogLevel) = 0;
        virtual ~executor_api() = default;
    protected:
        using ELString = ELString;
        using ELogLevel = ELogLevel;
        using LogLevel = LogLevel;
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


    using FLogPack = const LogPack&; // 格式化接受的数据包
    using FLString = std::string;    // 格式化输出的字符串

    struct format_api {
    protected:
        using FLogPack = FLogPack;
        using FLString = FLString;
        using LogLevel = LogLevel;
    };
    namespace format_apis {
        struct stdcxx20 : format_api {};
        struct traditional : format_api{};
    };

    template <typename Formatter, typename FormatAPI = format_api>
    using is_format_api = std::is_base_of<FormatAPI, Formatter>;
}

#endif //ZCLIBLOG_LOGGER_TYPES_HPP
