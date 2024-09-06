#include "zeus/foundation/resource/pipe_stream_buffer.h"

#ifdef __linux__
#include <unistd.h>
#include "zeus/foundation/core/posix/eintr_wrapper.h"

namespace zeus
{
struct PipeStreamBufferImpl
{
    int pipe;
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
    if (_impl->pipe > 0)
    {
        const auto writeLength = HANDLE_EINTR(write(_impl->pipe, buffer, length));
        if (writeLength)
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
    if (_impl->pipe > 0)
    {
        const auto readLength = HANDLE_EINTR(read(_impl->pipe, buffer, length));
        if (readLength > 0)
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