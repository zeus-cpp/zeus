#pragma once
#include <streambuf>
#include <memory>

namespace zeus
{
struct BasicStreamBufferImpl;
class BasicStreamBuffer : public std::streambuf
{
public:
    enum class BufferMode
    {
        Read,
        Write
    };
    BasicStreamBuffer(BufferMode mode, size_t bufferSize);
    ~BasicStreamBuffer();
    BasicStreamBuffer(const BasicStreamBuffer&)            = delete;
    BasicStreamBuffer(BasicStreamBuffer&& other)           = delete;
    BasicStreamBuffer& operator=(const BasicStreamBuffer&) = delete;
    BasicStreamBuffer& operator=(BasicStreamBuffer&&)      = delete;
protected:
    int_type overflow(int_type c) override;
    int_type underflow() override;
    int      sync() override;

    virtual int WriteDevice(const char_type* buffer, std::size_t length);
    virtual int ReadDevice(char_type* buffer, std::size_t length);
private:
    int FlushDevice();
private:
    std::unique_ptr<BasicStreamBufferImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
