#pragma once

#include <cstdint>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <string>

namespace zeus
{
class BaseDigest
{
public:
    BaseDigest();
    virtual ~BaseDigest();
    BaseDigest(const BaseDigest &)            = delete;
    BaseDigest &operator=(const BaseDigest &) = delete;
    BaseDigest(BaseDigest &&other) noexcept;
    BaseDigest &operator=(BaseDigest &&other) noexcept;

    virtual std::string      Name()   = 0;
    //获取目前已经散列的数据计算出的散列值(二进制)
    virtual const std::byte *Digest() = 0;

    //清空已经散列的数据,重置状态
    virtual void Reset() = 0;

    //添加一段数据进行散列计算
    virtual void Update(const void *input, size_t length);

    //添加一段字符串数据进行散列计算
    virtual void Update(const std::string &str);

    //添加一个文件内容数据进行散列计算
    virtual void Update(std::ifstream &in);

    virtual void Update(const std::filesystem::path &path, bool binary);

    //停止散列计算，获取目前已经散列的数据计算出的的字符串
    virtual std::string ToString(bool upCase = false);

    virtual size_t GetSize() = 0;

protected:
    virtual void UpdateImpl(const void *input, size_t length) = 0;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
