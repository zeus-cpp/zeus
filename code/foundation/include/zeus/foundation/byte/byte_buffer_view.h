#pragma once
// Package: Utils

#include <memory>
#include <string>

namespace zeus
{
struct ByteBufferViewImpl;
class ByteBufferView
{
public:
    ByteBufferView(const uint8_t *data, size_t size);
    ~ByteBufferView();
    ByteBufferView(const ByteBufferView &other) noexcept;
    ByteBufferView(ByteBufferView &&other) noexcept;
    ByteBufferView &operator=(const ByteBufferView &other) noexcept;
    ByteBufferView &operator=(ByteBufferView &&other) noexcept;
    const uint8_t  &operator[](size_t index) const;
    uint8_t        &operator[](size_t index);
    operator bool() const noexcept;
    const uint8_t *Data() const;
    uint8_t       *Data();
    size_t         Size() const;
    ByteBufferView SubView(size_t offset, size_t size) const;
private:
    std::unique_ptr<ByteBufferViewImpl> _impl;
};

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
