#pragma once
#include <streambuf>
#include "zeus/foundation/resource/basic_stream_buffer.h"
#include "zeus/foundation/core/platform_def.h"

namespace zeus
{
struct PipeStreamBufferImpl;
class PipeStreamBuffer : public BasicStreamBuffer
{
public:
    //PipeStreamBuffer 并不管理pipe的生命周期，调用者应保证pipe的生命周期大于PipeStreamBuffer
    PipeStreamBuffer(PlatformFileHandle pipe, BasicStreamBuffer::BufferMode mode, size_t bufferSize = 4096);
    ~PipeStreamBuffer();
    PipeStreamBuffer(const PipeStreamBuffer&)            = delete;
    PipeStreamBuffer(PipeStreamBuffer&& other)           = delete;
    PipeStreamBuffer& operator=(const PipeStreamBuffer&) = delete;
    PipeStreamBuffer& operator=(PipeStreamBuffer&&)      = delete;
protected:
    int WriteDevice(const char_type* buffer, std::size_t length) override;
    int ReadDevice(char_type* buffer, std::size_t length) override;
private:
    std::unique_ptr<PipeStreamBufferImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
