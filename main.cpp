#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/logger_async.hpp"

#include "ZCLibLog/formatters/format.hpp"
#include "ZCLibLog/executors/cstdout.hpp"
#include "ZCLibLog/executors/iostream.hpp"

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

RingBuffer<std::string> buf(5);
inline ZCLibLog::executor& ringbuf() {
    using namespace ZCLibLog;
    static executor inst = [](ELString msg, ELogLevel) {
        buf.push(msg);
    };
    return inst;
}

ZCLibLog::LoggerSync<ZCLibLog::formatters::format> Logger{
    "MainLogger",
    {
        ZCLibLog::executors::iostream(),
        ringbuf()
    }
};


int main() {
    Logger.ALL("Hello {}!", ZCLibLog::PROJECT_NAME);

    Logger.TRACE("This is trace");

    Logger.DEBUG("This is debug");

    Logger.INFO("This is info");

    Logger.WARN("This is warning");

    Logger.ERROR("This is error");

    Logger.FATAL("This is fatal");

    return 0;
}