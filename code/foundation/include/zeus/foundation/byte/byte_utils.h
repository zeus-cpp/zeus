#pragma once
// Package: Utils

#include <vector>
#include <utility>
#include <memory>
#include <cstdint>
#include <string>
#include <optional>
#include "zeus/foundation/byte/byte_buffer_view.h"

namespace zeus
{

std::vector<ByteBufferView> ByteSplit(const ByteBufferView &src, const ByteBufferView &delim, size_t stride = 1);

std::vector<uint8_t>   ByteJoin(std::vector<ByteBufferView> &src, const ByteBufferView &delim);
std::string::size_type ByteFind(const ByteBufferView &src, const ByteBufferView &sub, size_t offset = 0, size_t stride = 1);
std::string::size_type ByteReverseFind(const ByteBufferView &src, const ByteBufferView &sub, size_t offset = 0, size_t stride = 1);
std::vector<uint8_t>   ByteReplace(const ByteBufferView &src, const ByteBufferView &sub, const ByteBufferView &replacement, size_t stride = 1);

bool                                ByteEndWith(const ByteBufferView &src, const ByteBufferView &end);
bool                                ByteStartWith(const ByteBufferView &src, const ByteBufferView &start);
std::optional<std::vector<uint8_t>> HexStringToBytes(const std::string &hex);

size_t CountLeftZero(uint32_t x);
size_t CountLeftOne(uint32_t x);
size_t CountRightZero(uint32_t x);
size_t CountRightOne(uint32_t x);

size_t CountLeftZero(uint16_t x);
size_t CountLeftOne(uint16_t x);
size_t CountRightZero(uint16_t x);
size_t CountRightOne(uint16_t x);

size_t CountLeftZero(uint8_t x);
size_t CountLeftOne(uint8_t x);
size_t CountRightZero(uint8_t x);
size_t CountRightOne(uint8_t x);

size_t CountZero(uint8_t x);
size_t CountOne(uint8_t x);
size_t CountZero(uint16_t x);
size_t CountOne(uint16_t x);
size_t CountZero(uint32_t x);
size_t CountOne(uint32_t x);

size_t CountZero(const ByteBufferView &src);
size_t CountOne(const ByteBufferView &src);
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
