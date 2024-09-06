#pragma once

#include <cstdint>

#include <memory>
#include <vector>

#include <zeus/expected.hpp>
#include <zeus/foundation/core/system_error.h>

namespace zeus
{

enum class SerializerError
{
    kFileIoError = 1,

    kDigestFileSizeUnsatisfied,
    kDigestNameUnmatched,
    kDigestValidationFailed,

    kCryptNameUnmatched,
    kCryptFileSizeUnsatisfied,
    kCryptEncryptError,
    kCryptDecryptError,
};

class Serializer
{
public:
    Serializer();
    virtual ~Serializer();
    Serializer(const Serializer&)            = delete;
    Serializer& operator=(const Serializer&) = delete;
    Serializer(Serializer&&)                 = delete;
    Serializer& operator=(Serializer&&)      = delete;

    virtual zeus::expected<std::vector<uint8_t>, SerializerError> Load() = 0;

    virtual zeus::expected<void, SerializerError> Save(const void* buffer, size_t bufferSize) = 0;
    zeus::expected<void, SerializerError>         Save(std::vector<uint8_t> const& data) { return Save(data.data(), data.size()); }
};

}

#include "zeus/foundation/core/zeus_compatible.h"
