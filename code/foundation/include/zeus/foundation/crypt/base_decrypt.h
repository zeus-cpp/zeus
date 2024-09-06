#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <cstddef>
#include <filesystem>

namespace zeus
{
class BaseDecrypt
{
public:
    BaseDecrypt();
    virtual ~BaseDecrypt();
    BaseDecrypt(const BaseDecrypt &)            = delete;
    BaseDecrypt &operator=(const BaseDecrypt &) = delete;
    BaseDecrypt(BaseDecrypt &&other) noexcept;
    BaseDecrypt &operator=(BaseDecrypt &&other) noexcept;

    virtual std::string      Name()      = 0;
    virtual const std::byte *PlainText() = 0;
    virtual void             Reset()     = 0;
    virtual void             Update(const void *input, size_t length);
    virtual void             Update(const std::string &str);
    virtual void             Update(std::ifstream &in);
    virtual void             Update(const std::filesystem::path &path, bool binary);
    virtual std::string      GetString();
    virtual size_t           GetSize() = 0;
    virtual void             End()     = 0;

protected:
    virtual void UpdateImpl(const void *input, size_t length) = 0;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
