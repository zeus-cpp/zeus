#pragma once

#include <string>
#include <memory>
#include <cstddef>
#include <cryptopp/cryptlib.h>

namespace zeus
{

class BaseDigestImpl
{
public:

    BaseDigestImpl(std::shared_ptr<CryptoPP::HashTransformation> algorithm);

    virtual ~BaseDigestImpl();

    //获取目前已经散列的数据计算出的散列值(二进制)
    virtual const std::byte* Digest();

    //清空已经散列的数据,重置状态
    virtual void Reset();

    virtual size_t GetSize();
    virtual void   UpdateImpl(const void* input, size_t length);
protected:
    std::shared_ptr<CryptoPP::HashTransformation> _algorithm;
    std::unique_ptr<CryptoPP::byte[]>             _digest; /* message digest */
private:
    bool _finished;                                        /* calculate finished ? */
};
}
