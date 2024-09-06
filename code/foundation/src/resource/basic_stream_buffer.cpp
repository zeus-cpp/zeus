#include "zeus/foundation/resource/basic_stream_buffer.h"
#include <vector>
#include <cassert>

namespace zeus
{
struct BasicStreamBufferImpl
{
    size_t                        bufferSize = 0;
    std::vector<char>             buffer;
    bool                          eof         = false;
    BasicStreamBuffer::BufferMode mode        = BasicStreamBuffer::BufferMode::Read;
    static const size_t           putbackSize = 8; //用于准备回退的缓冲空间
};

BasicStreamBuffer::BasicStreamBuffer(BufferMode mode, size_t bufferSize) : _impl(std::make_unique<BasicStreamBufferImpl>())
{
    assert(!_impl->eof);
    _impl->mode       = mode;
    _impl->bufferSize = bufferSize;
    if (_impl->bufferSize <= _impl->putbackSize)
    {
        //总缓冲空间不能小于回退缓冲空间
        _impl->bufferSize = _impl->putbackSize * 2;
    }
    _impl->buffer = std::vector<char>(_impl->bufferSize);
    this->setg(_impl->buffer.data() + _impl->putbackSize, _impl->buffer.data() + _impl->putbackSize, _impl->buffer.data() + _impl->putbackSize);
    this->setp(_impl->buffer.data(), _impl->buffer.data() + _impl->bufferSize);
}

BasicStreamBuffer::~BasicStreamBuffer()
{
}
std::streambuf::int_type BasicStreamBuffer::overflow(int_type c)
{
    if (BasicStreamBuffer::BufferMode::Read == _impl->mode)
    {
        return std::char_traits<char>::eof();
    }

    if (FlushDevice() < 0)
    {
        return std::char_traits<char>::eof();
    }
    if (c != std::char_traits<char>::eof())
    {
        *this->pptr() = std::char_traits<char>::to_char_type(c);
        this->pbump(1);
    }

    return c;
}
std::streambuf::int_type BasicStreamBuffer::underflow()
{
    if (BasicStreamBuffer::BufferMode::Write == _impl->mode)
    {
        return std::char_traits<char>::eof();
    }

    if (this->gptr() && (this->gptr() < this->egptr()))
    {
        return std::char_traits<char>::to_int_type(*this->gptr());
    }

    int putback = static_cast<int>(this->gptr() - this->eback());
    if (putback > static_cast<int>(_impl->putbackSize))
    {
        //不支持更大的回退了
        putback = _impl->putbackSize;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)  没有c++20，无法使用std::span
    std::char_traits<char>::move(_impl->buffer.data() + (_impl->putbackSize - putback), this->gptr() - putback, putback);

    int n = ReadDevice(_impl->buffer.data() + _impl->putbackSize, _impl->bufferSize - _impl->putbackSize);
    if (n <= 0)
    {
        return std::char_traits<char>::eof();
    }

    this->setg(
        _impl->buffer.data() + (_impl->putbackSize - putback), _impl->buffer.data() + _impl->putbackSize,
        _impl->buffer.data() + _impl->putbackSize + n
    );

    // return next character
    return std::char_traits<char>::to_int_type(*this->gptr());
}
int BasicStreamBuffer::sync()
{
    if (this->pptr() && this->pptr() > this->pbase())
    {
        if (FlushDevice() < 0)
        {
            return -1;
        }
    }
    return 0;
}
int BasicStreamBuffer::WriteDevice(const char_type* /*buffer*/, std::size_t /*length*/)
{
    return 0;
}
int BasicStreamBuffer::ReadDevice(char_type* /*buffer*/, std::size_t /*length*/)
{
    return 0;
}
int BasicStreamBuffer::FlushDevice()
{
    const int n = static_cast<int>((this->pptr() - this->pbase()));
    if (WriteDevice(this->pbase(), n) == n)
    {
        this->pbump(-n);
        return n;
    }
    return -1;
}
} // namespace zeus