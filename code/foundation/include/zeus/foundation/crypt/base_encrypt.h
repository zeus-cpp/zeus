#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstddef>

namespace zeus
{
class BaseEncrypt
{
public:

    BaseEncrypt();
    virtual ~BaseEncrypt();
    BaseEncrypt(const BaseEncrypt &)            = delete;
    BaseEncrypt &operator=(const BaseEncrypt &) = delete;
    BaseEncrypt(BaseEncrypt &&other) noexcept;
    BaseEncrypt &operator=(BaseEncrypt &&other) noexcept;

    virtual std::string      Name()       = 0;
    //CipherText方法只能得到当前已经产生的密文，但是不能作为最终的密文，因为大对数的加密算法会要求在明文的末尾进行对齐填充
    virtual const std::byte *CipherText() = 0;
    //重置加密器状态
    virtual void             Reset()      = 0;
    //输入明文
    virtual void             Update(const void *input, size_t length);
    virtual void             Update(const std::string &str);
    virtual void             Update(std::ifstream &in);
    virtual void             Update(const std::filesystem::path &path, bool binary);
    //此方法会自动调用内部会自动调用End进行明文的末尾对齐填充
    virtual std::string      GetString();
    //CipherText方法只能得到当前已经产生的密文长度，但是不能作为最终的密文长度，因为大对数的加密算法会要求在明文的末尾进行对齐填充
    virtual size_t           GetSize() = 0;
    //当全部明文已经输入后应该调用End方法，对明文的末尾进行对齐填充，调用此方法后不应该在继续使用Update方法输入明文
    virtual void             End()     = 0;

protected:
    virtual void UpdateImpl(const void *input, size_t length) = 0;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
