#include "zeus/foundation/byte/byte_buffer_view.h"
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic) 没有c++20，没有std::span可用
#include <cassert>
namespace zeus
{
struct ByteBufferViewImpl
{
    uint8_t* data = nullptr;
    size_t   size = 0;
};

ByteBufferView::ByteBufferView(const uint8_t* data, size_t size) : _impl(std::make_unique<ByteBufferViewImpl>())
{
    _impl->data = const_cast<uint8_t*>(data);
    _impl->size = size;
}
ByteBufferView::~ByteBufferView()
{
}
ByteBufferView::ByteBufferView(const ByteBufferView& other) noexcept : _impl(std::make_unique<ByteBufferViewImpl>(*other._impl))
{
}
ByteBufferView::ByteBufferView(ByteBufferView&& other) noexcept : _impl(std::move(other._impl))
{
}

ByteBufferView& ByteBufferView::operator=(const ByteBufferView& other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}
ByteBufferView& ByteBufferView::operator=(ByteBufferView&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
const uint8_t* ByteBufferView::Data() const
{
    return _impl->data;
}

uint8_t* ByteBufferView::Data()
{
    return _impl->data;
}

size_t ByteBufferView::Size() const
{
    return _impl->size;
}

const uint8_t& ByteBufferView::operator[](size_t index) const
{
    assert(_impl->data);
    return _impl->data[index];
}

uint8_t& ByteBufferView::operator[](size_t index)
{
    assert(_impl->data);
    return _impl->data[index];
}

ByteBufferView::operator bool() const noexcept
{
    return _impl->data != nullptr && _impl->size;
}

ByteBufferView ByteBufferView::SubView(size_t offset, size_t size) const
{
    assert(_impl->data);
    return {_impl->data + offset, size};
}

} // namespace zeus
  // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic) 没有c++20，没有std::span可用