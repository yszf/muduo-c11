#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include "muduo-c11/base/noncopyable.h"
#include "muduo-c11/base/Types.h"

namespace muduo {

    namespace detail {

        const int kSmallBuffer = 4000;
        const int kLargeBuffer = 4000 * 1000;

        template<int SIZE>
        class FixedBuffer : noncopyable {
        public:
            FixedBuffer()
                : cur_(data_) {
                
            }

            ~FixedBuffer() {

            }

            void append(const char* buf, size_t len) {
                memcpy(cur_, buf, len);
                cur_ += len;
            }

            const char* data() const {
                return data_;
            }

            int length() const {
                return static_cast<int>(cur_ - data_);
            }

            char* current() {
                return cur_;
            }

            int avail() const {
                return static_cast<int>(end() - current());
            }

            void add(size_t len) {
                cur_ += len;
            }

            void reset() {
                cur_ = data_;
            }

            void bzero() {
                memZero(data_, sizeof data);
            }

            void setCookie(void (*cookie)()) {
                cookie_ = cookie;
            }

            string toString() const {
                return string(data_, length());
            }

        private:
            const char* end() const {
                return data_ + sizeof data_;
            }

            void (*cookie_)();

            char data_[SIZE];
            char* cur_;
            
        }; // template class FixedBuffer
    }

    class LogStream : noncopyable {
        typedef LogStream self;
    public:
        typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

        self& operator<<(bool v) {
            buffer_.append(v ? "1" : "0", 1);
            return *this;
        }

        self& operator<<(short);
        self& operator<<(unsigned short);
        self& operator<<(int);
        self& operator<<(unsigned int);
        self& operator<<(long);
        self& operator<<(unsigned long);
        self& operator<<(long long);
        self& operator<<(unsigned long long);

        self& operator<<(const void*);

        self& operator<<(float v) {
            *this << static_cast<double>(v);
            return *this;
        }
        self operator<<(double);

        self& operator<<(char v) {
            buffer_.append(&v, 1);
        }

        self& operator<<(const char* str) {
            if (str) {
                buffer_.append(str, strlen(str));
            }
            else {
                buffer_.append("(null)", 6);
            }
            return *this;
        }

        self& operator<<(const unsigned char* str) {
            return operator<<(reinterpret_cast<const char*>(str));
        }

        self& operator<<(const string& v) {
            buffer_.append(v.c_str(), v.size());
            return *this;
        }

        void append(const char* data, int len) {
            buffer_.append(data, len);
        }

        const Buffer& buffer() const {
            return buffer_;
        }

        void resetBuffer() {
            buffer_.reset();
        }

    private:
        Buffer buffer_;

    }; // class LogStream

} // namespace muduo

#endif // MUDUO_BASE_LOGSTREAM_H