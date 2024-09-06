#pragma once

#include <cstdint>
#include <cstddef>

/* MD5 declaration. */

namespace zeus
{
class RawMd5DigestImpl
{
public:
    RawMd5DigestImpl();
    void             UpdateImpl(const void *input, size_t length);
    const std::byte *Digest();
    void             Reset();
    size_t           GetSize();
private:
    void Update(const std::byte *input, size_t length);
    void Final();
    void Transform(const std::byte block[64]);
    void Encode(const uint32_t *input, std::byte *output, size_t length);
    void Decode(const std::byte *input, uint32_t *output, size_t length);

private:
    uint32_t  _state[4];   /* state (ABCD) */
    uint32_t  _count[2];   /* number of bits, modulo 2^64 (low-order word first) */
    std::byte _buffer[64]; /* input buffer */
    std::byte _digest[16]; /* message digest */
    bool      _finished;   /* calculate finished ? */

    static const size_t BUFFER_SIZE = 1024;
};
} // namespace zeus
