#include "zeus/foundation/resource/pipe_stream_buffer.h"

#ifdef _WIN32
#include <Windows.h>

namespace zeus
{
struct PipeStreamBufferImpl
{
    HANDLE pipe;
};
PipeStreamBuffer::PipeStreamBuffer(PlatformFileHandle pipe, BasicStreamBuffer::BufferMode mode, size_t bufferSize)
    : BasicStreamBuffer(mode, bufferSize), _impl(std::make_unique<PipeStreamBufferImpl>())
{
    _impl->pipe = pipe;
}

PipeStreamBuffer::~PipeStreamBuffer()
{
}

int PipeStreamBuffer::WriteDevice(const char_type* buffer, std::size_t length)
{
    if (_impl->pipe && INVALID_HANDLE_VALUE != _impl->pipe)
    {
        DWORD      writeLength = 0;
        const bool ret         = TRUE == WriteFile(_impl->pipe, buffer, length, &writeLength, nullptr);
        if (ret)
        {
            return static_cast<int>(writeLength);
        }
        else
        {
            return -1;
        }
    }
    return 0;
}

int PipeStreamBuffer::ReadDevice(char_type* buffer, std::size_t length)
{
    if (_impl->pipe && INVALID_HANDLE_VALUE != _impl->pipe)
    {
        DWORD      readLength = 0;
        const bool ret        = TRUE == ReadFile(_impl->pipe, buffer, length, &readLength, nullptr);
        if (ret)
        {
            return static_cast<int>(readLength);
        }
        else
        {
            return -1;
        }
    }
    return 0;
}
} // namespace zeus
#endif