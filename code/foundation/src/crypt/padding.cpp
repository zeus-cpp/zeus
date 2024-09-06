#include "zeus/foundation/crypt/padding.h"

namespace zeus
{

std::size_t FillPKCS7Padding(uint8_t* buffer, std::size_t bufferLength, std::size_t dataLength, uint8_t blockLength)
{
    if (0 == blockLength)
    {
        return 0;
    }
    uint8_t paddingLength = blockLength - dataLength % blockLength;
    if (0 == paddingLength)
    {
        paddingLength = blockLength;
    }
    if (bufferLength < paddingLength)
    {
        return 0;
    }
    for (auto i = 1; i <= paddingLength; i++)
    {
        buffer[paddingLength - i] = paddingLength;
    }
    return paddingLength;
}

std::size_t FindPKCS7Padding(const uint8_t* buffer, std::size_t bufferLength)
{
    if (bufferLength)
    {
        auto paddingLength = buffer[bufferLength - 1];

        if (bufferLength >= paddingLength)
        {
            bool check = true;
            for (uint8_t i = 1; i <= paddingLength; i++)
            {
                if (paddingLength != buffer[bufferLength - i])
                {
                    check = false;
                    break;
                }
            }
            if (check)
            {
                return paddingLength;
            }
        }
    }
    return 0;
}

std::size_t FillANSI_X_923Padding(uint8_t* buffer, std::size_t bufferLength, std::size_t dataLength, uint8_t blockLength)
{
    if (0 == blockLength)
    {
        return 0;
    }
    uint8_t paddingLength = blockLength - dataLength % blockLength;
    if (0 == paddingLength)
    {
        paddingLength = blockLength;
    }
    if (bufferLength < paddingLength)
    {
        return 0;
    }
    for (auto i = 2; i <= paddingLength; i++)
    {
        buffer[paddingLength - i] = 0;
    }
    buffer[paddingLength - 1] = paddingLength;
    return paddingLength;
}

std::size_t FindANSI_X_923Padding(const uint8_t* buffer, std::size_t bufferLength)
{
    if (bufferLength)
    {
        auto paddingLength = buffer[bufferLength - 1];

        if (bufferLength >= paddingLength)
        {
            bool check = true;
            for (uint8_t i = 2; i <= paddingLength; i++)
            {
                if (0 != buffer[bufferLength - i])
                {
                    check = false;
                    break;
                }
            }
            if (check)
            {
                return paddingLength;
            }
        }
    }
    return 0;
}

std::size_t FillISO_10126Padding(uint8_t* buffer, std::size_t bufferLength, std::size_t dataLength, uint8_t blockLength)
{
    if (0 == blockLength)
    {
        return 0;
    }
    uint8_t paddingLength = blockLength - dataLength % blockLength;
    if (0 == paddingLength)
    {
        paddingLength = blockLength;
    }
    if (bufferLength < paddingLength)
    {
        return 0;
    }
    for (auto i = 2; i <= paddingLength; i++)
    {
        buffer[paddingLength - i] = static_cast<uint8_t>(dataLength % i);
    }
    buffer[paddingLength - 1] = paddingLength;
    return paddingLength;
}

std::size_t FindISO_10126Padding(const uint8_t* buffer, std::size_t bufferLength)
{
    if (bufferLength)
    {
        auto paddingLength = buffer[bufferLength - 1];

        if (bufferLength >= paddingLength)
        {
            return paddingLength;
        }
    }
    return 0;
}
} // namespace zeus