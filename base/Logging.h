#ifndef MUDUO_BASE_LOGGING_H
#define MUDUO_BASE_LOGGING_H

#include "muduo-c11/base/Timestamp.h"
#include "muduo-c11/base/LogStream.h"

namespace muduo {

    class TimeZone;

    class Logger {
    public:
        enum LogLevel {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        class SourceFile {
        public:
            template<int N>
            SourceFile(const char (&arr)[N])
                : data_(arr),
                  size_(N - 1) {
                const char* last = strrchr(data_, '/');
                if (last) {
                    data_ = last + 1;
                    size_ -= static_cast<int>(data_ - arr);
                }
            }

            explicit SourceFile(const char* filename) 
                : data_(filename) {
                const char* last = strrchr(data_, '/');
                if (last) {
                    data_ = last + 1;
                }
                size_ = static_cast<int>(strlen(data_));
            }

            const char* data_;
            int size_;

        }; // class SourceFile

        Logger(SourceFile file, int line);
        Logger(SourceFile file, int line, LogLevel level);
        Logger(SourceFile file, int line, LogLevel level, const char* func);
        Logger(SourceFile file, int line, bool toAbort);
        ~Logger();

        LogStream& stream() {
            return impl_.stream_;
        }

        static LogLevel logLevel();
        static void setLogLevel(LogLevel level);

        typedef void (*OutputFunc)(const char* msg, int len);
        typedef void (*FlushFunc)();
        static void setOutput(OutputFunc);
        static void setFlush(FlushFunc);

        static void setTimeZone(const TimeZone& tz);

    private:
        class Impl {
        public:
            typedef Logger::LogLevel LogLevel;
            Impl(LogLevel level, int old_errno, const SourceFile& file, int line);

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;
        }; // class Impl

        Impl impl_;

    }; // class Logger

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel() {
        return g_logLevel;
    }

} // namespace muduo

#endif // MUDUO_BASE_LOGGING_H
