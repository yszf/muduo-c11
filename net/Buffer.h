#ifndef MUDUO_NET_BUFFER_H
#define MUDUO_NET_BUFFER_H

#include "muduo-c11/base/copyable.h"
#include "muduo-c11/base/Types.h"
#include "Endian.h"

#include <vector>

namespace muduo {

    namespace net {

        class Buffer : public copyable{
        public:
            static const size_t kCheapPrepend = 8;
            static const size_t kInitialSize = 1024;

            explicit Buffer(size_t initialSize = kInitialSize) 
                : buffer_(kCheapPrepend + kInitialSize),
                readerIndex_(kCheapPrepend), 
                writerIndex_(kCheapPrepend) {
                assert(readableBytes() == 0);
                assert(writableBytes() == initialSize);
                assert(prependableBytes() == kCheapPrepend);
            }

            ~Buffer() {}

            size_t readableBytes() const {
                return writerIndex_ - readerIndex_;
            }

            size_t writableBytes() const {
                return buffer_.size() - writerIndex_;
            }

            size_t prependableBytes() const {
                return readerIndex_;
            }

            const char* peek() const {
                return begin() + readerIndex_;
            }

            void retrieve(size_t len) {
                assert(len <= readableBytes());
                if (len < readableBytes()) {
                    readerIndex_ += len;
                }
                else {
                    retrieveAll();
                }
            }

            void retrieveUtil(const char* end) {
                assert(peek() <= end);
                assert(end <= beginWrite());
                retrieve(end - peek());
            }

            void retrieveInt64() {
                retrieve(sizeof(int64_t));
            }

            void retrieveInt32() {
                retrieve(sizeof(int32_t));
            }

            void retrieveInt16() {
                retrieve(sizeof(int16_t));
            }

            void retrieveInt8() {
                retrieve(sizeof(int8_t));
            }

            void retrieveAll() {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            string retrieveAllAsString() {
                return retrieveAsString(readableBytes());
            }

            string retrieveAsString(size_t len) {
                assert(len <= readableBytes());
                string result(peek(), len);
                retrieve(len);
                return result;
            }

            void append(const char* data, size_t len) {
                ensureWritableBytes(len);
                std::copy(data, data + len, beginWrite());
                hasWritten(len);
            }

            void append(const void* data, size_t len) {
                append(static_cast<const char*>(data), len);
            }

            void ensureWritableBytes(size_t len) {
                if (writableBytes() < len) {
                    makeSpace(len);
                }
                assert(writableBytes() >= len);
            }

            char* beginWrite() {
                return begin() + writerIndex_;
            }

            void hasWritten(size_t len) {
                assert(len <= writableBytes());
                writerIndex_ += len;
            }

            void unwrite(size_t len) {
                assert(len <= readableBytes());
                writerIndex_ -= len;
            }

            void appendInt64(int64_t x) {
                int64_t be64 = sockets::hostToNetwork64(x);
                append(&be64, sizeof be64);
            }

            void appendInt32(int32_t x) {
                int32_t be32 = sockets::hostToNetwork32(x);
                append(&be32, sizeof be32);
            }

            void appendInt16(int16_t x) {
                int16_t be16 = sockets::hostToNetwork16(x);
                append(&be16, sizeof be16);
            }

            void appendInt8(int8_t x) {
                append(&x, sizeof x);
            }

            ssize_t readFd(int fd, int* savedErrno);

        private:
            char* begin() {
                return &*buffer_.begin();
            }

            const char* begin() const {
                return &*buffer_.begin();
            }

            void makeSpace(size_t len) {
                if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
                    buffer_.resize(writerIndex_ + len);
                }
                else {
                    // move readable data to the front, make space inside buffer
                    assert(kCheapPrepend < readerIndex_);
                    size_t readable = readableBytes();
                    std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
                    readerIndex_ = kCheapPrepend;
                    writerIndex_ = readerIndex_ + readable;
                    assert(readable == readableBytes());
                }
            }

            std::vector<char> buffer_;
            size_t readerIndex_;
            size_t writerIndex_;

            static const char kCRLF[];
        };

    } // namespace net

} // namespace muduo

#endif // MUDUO_NET_BUFFER_H