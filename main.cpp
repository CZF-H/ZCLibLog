#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/logger_async.hpp"

#include "ZCLibLog/formatters/vformat.hpp"
#include "ZCLibLog/executors/cstdout.hpp"
#include "ZCLibLog/executors/iostream.hpp"
#include "ZCLibLog/executors/ostream.hpp"

template<typename T>
class RingBuffer {
    std::vector<T> buffer;
    size_t head{};
    size_t tail{};
    bool full{};

public:
    explicit RingBuffer(size_t capacity)
        : buffer(capacity) {
        if (capacity == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    using value_type = T;

    class iterator {
        const RingBuffer* rb;
        size_t pos;
        size_t count;

    public:
        iterator(const RingBuffer* rb, size_t pos, size_t count)
            : rb(rb), pos(pos), count(count) {}

        const T& operator*() const { return rb->buffer[pos]; }
        iterator& operator++() {
            pos = (pos + 1) % rb->buffer.size();
            ++count;
            return *this;
        }
        bool operator!=(const iterator& other) const {
            return count != other.count;
        }
    };

    [[nodiscard]] iterator begin() const { return iterator(this, head, 0); }
    [[nodiscard]] iterator end() const { return iterator(this, tail, size()); }

    [[nodiscard]] size_t size() const { return full ? buffer.size() : (tail + buffer.size() - head) % buffer.size(); }

    void push(const T& item) {
        buffer[tail] = item;
        tail = (tail + 1) % buffer.size();
        if (full) head = (head + 1) % buffer.size();
        if (tail == head) full = true;
    }

    void push(T&& item) {
        buffer[tail] = std::move(item);
        tail = (tail + 1) % buffer.size();
        if (full) head = (head + 1) % buffer.size();
        if (tail == head) full = true;
    }

    [[nodiscard]] bool empty() const { return (!full && head == tail); }

    void clear() {
        head = tail = 0;
        full = false;
    }
};

struct format {
    using format_api = ZCLibLog::cxx20_format_api;
    template <typename... Args>
    static std::string do_format(ZCLibLog::FLogPack pack, std::format_string<Args...> fmt, Args&&... args) {
        using namespace ZCLibLog;

        std::string f_msg;
        if (sizeof...(args) == 0) {
            f_msg = fmt.get();
        }
        else {
            try {
                f_msg = std::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args&&>(args)...);
            } catch (const std::format_error&) {
                return {};
            }
        }

        const auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(pack.time));

        const char* level_str;
        switch (pack.level) {
            case LogLevel_TRACE: level_str = "[TRACE]";
                break;
            case LogLevel_DEBUG: level_str = "[DEBUG]";
                break;
            case LogLevel_INFO: level_str = "[INFO]";
                break;
            case LogLevel_WARN: level_str = "[WARN]";
                break;
            case LogLevel_ERROR: level_str = "[ERROR]";
                break;
            case LogLevel_FATAL: level_str = "[FATAL]";
                break;
            default: level_str = "[INFO]";
                break;
        }

        return std::format(
            "{:%Y-%m-%d %H:%M:%S} [{}] {} {}",
            std::chrono::time_point_cast<std::chrono::milliseconds>(tp),
            *pack.module,
            level_str,
            f_msg
        );
    }
};

ZCLibLog::LoggerSync<format> Logger{
    "MainLogger",
    {
        ZCLibLog::executors::cstdout()
    }
};
RingBuffer<std::string> buf(5);

inline ZCLibLog::executor& ringbuf() {
    using namespace ZCLibLog;
    static executor inst = [](ELString msg, ELogLevel) {
        buf.push(msg);
    };
    return inst;
}

int main() {
    const auto start = std::chrono::high_resolution_clock::now();

    Logger.INFO("Hello {}", ZCLibLog::PROJECT_NAME);
    Logger.INFO("Hello {}", ZCLibLog::PROJECT_NAME);

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Used: " << duration.count() << " ns\n";

    return 0;
}
