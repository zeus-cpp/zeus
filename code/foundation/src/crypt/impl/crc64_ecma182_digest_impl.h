#pragma once

#include <cstdint>
#include <cstddef>

namespace zeus
{

class Crc64Ecma182DigestImpl
{
public:

    /* Default construct. */
    Crc64Ecma182DigestImpl();

    ~Crc64Ecma182DigestImpl();

    const std::byte* Digest();

    //清空已经散列的数据,重置状态
    void Reset();

    size_t GetSize();
    void   UpdateImpl(const void* input, size_t length);
private:
    uint64_t _hash;
};
}
