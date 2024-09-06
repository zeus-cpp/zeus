#include "zeus/foundation/core/random.h"
#include <cstdlib>
#include <cmath>
#include <string>
#include <ctime>
#include <mutex>
#include <limits>
#include <random>

namespace zeus
{

#define AUTO_INTEGER_RAND_GENERATE(Name, Type)                                                                                \
    Type Rand##Name()                                                                                                         \
    {                                                                                                                         \
        thread_local std::mt19937           generator(std::random_device {}());                                               \
        std::uniform_int_distribution<Type> distribution(std::numeric_limits<Type>::min(), std::numeric_limits<Type>::max()); \
        return distribution(generator);                                                                                       \
    }                                                                                                                         \
    Type Rand##Name(Type max)                                                                                                 \
    {                                                                                                                         \
        thread_local std::mt19937           generator(std::random_device {}());                                               \
        std::uniform_int_distribution<Type> distribution(std::numeric_limits<Type>::min(), max);                              \
        return distribution(generator);                                                                                       \
    }                                                                                                                         \
    Type Rand##Name(Type min, Type max)                                                                                       \
    {                                                                                                                         \
        thread_local std::mt19937           generator(std::random_device {}());                                               \
        std::uniform_int_distribution<Type> distribution(min, max);                                                           \
        return distribution(generator);                                                                                       \
    }

#define AUTO_BYTE_RAND_GENERATE(Name, Type)                                                                                  \
    Type Rand##Name()                                                                                                        \
    {                                                                                                                        \
        thread_local std::mt19937          generator(std::random_device {}());                                               \
        std::uniform_int_distribution<int> distribution(std::numeric_limits<Type>::min(), std::numeric_limits<Type>::max()); \
        return static_cast<Type>(distribution(generator));                                                                   \
    }                                                                                                                        \
    Type Rand##Name(Type max)                                                                                                \
    {                                                                                                                        \
        thread_local std::mt19937          generator(std::random_device {}());                                               \
        std::uniform_int_distribution<int> distribution(std::numeric_limits<Type>::min(), max);                              \
        return static_cast<Type>(distribution(generator));                                                                   \
    }                                                                                                                        \
    Type Rand##Name(Type min, Type max)                                                                                      \
    {                                                                                                                        \
        thread_local std::mt19937          generator(std::random_device {}());                                               \
        std::uniform_int_distribution<int> distribution(min, max);                                                           \
        return static_cast<Type>(distribution(generator));                                                                   \
    }

#define AUTO_REAL_RAND_GENERATE(Name, Type)                                                                                    \
    Type Rand##Name()                                                                                                          \
    {                                                                                                                          \
        thread_local std::mt19937            generator(std::random_device {}());                                               \
        std::uniform_real_distribution<Type> distribution(std::numeric_limits<Type>::min(), std::numeric_limits<Type>::max()); \
        return distribution(generator);                                                                                        \
    }                                                                                                                          \
    Type Rand##Name(Type max)                                                                                                  \
    {                                                                                                                          \
        thread_local std::mt19937            generator(std::random_device {}());                                               \
        std::uniform_real_distribution<Type> distribution(std::numeric_limits<Type>::min(), max);                              \
        return distribution(generator);                                                                                        \
    }                                                                                                                          \
    Type Rand##Name(Type min, Type max)                                                                                        \
    {                                                                                                                          \
        thread_local std::mt19937            generator(std::random_device {}());                                               \
        std::uniform_real_distribution<Type> distribution(min, max);                                                           \
        return distribution(generator);                                                                                        \
    }

AUTO_BYTE_RAND_GENERATE(Uint8, uint8_t)

AUTO_BYTE_RAND_GENERATE(Int8, int8_t)

AUTO_INTEGER_RAND_GENERATE(Uint16, uint16_t)

AUTO_INTEGER_RAND_GENERATE(Int16, int16_t)

AUTO_INTEGER_RAND_GENERATE(Uint32, uint32_t)

AUTO_INTEGER_RAND_GENERATE(Int32, int32_t)

AUTO_INTEGER_RAND_GENERATE(Uint64, uint64_t)

AUTO_INTEGER_RAND_GENERATE(Int64, int64_t)

AUTO_REAL_RAND_GENERATE(Float, float)

AUTO_REAL_RAND_GENERATE(Double, double)

bool RandBool()
{
    return static_cast<bool>(RandUint8() % 2);
}

char RandLetter()
{
    static const char charset[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ"};
    return charset[RandUint32() % (sizeof(charset) - 1)];
}
char RandLowerLetter()
{
    static const char charset[] = {"abcdefghijklmnopqrstuvwxyz"};
    return charset[RandUint32() % (sizeof(charset) - 1)];
}
char RandUpperLetter()
{
    static const char charset[] = {"ABCDEFGHIJKLMNOPQRSTUVWZYZ"};
    return charset[RandUint32() % (sizeof(charset) - 1)];
}
char RandNumberLetter()
{
    static const char charset[] = {"0123456789"};
    return charset[RandUint32() % (sizeof(charset) - 1)];
}
char RandHex()
{
    static const char charset[] = {"0123456789ABCDEF"};
    return charset[RandUint32() % (sizeof(charset) - 1)];
}
void RandBytes(void* output, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        *(static_cast<char*>(output) + i) = RandInt8();
    }
}
std::string RandWord()
{
    auto size = RandUint8(5, 15);
    return RandString(size);
}
std::string RandString(size_t size)
{
    std::string stream;
    stream.reserve(size);
    for (size_t i = 0; i < size; i++)
    {
        stream.push_back(RandLetter());
    }
    return stream;
}

std::string RandIpV4String()
{
    return std::to_string(RandUint8(1, 255)) + "." + std::to_string(RandUint8(0, 255)) + "." + std::to_string(RandUint8(0, 255)) + "." +
           std::to_string(RandUint8(1, 254));
}
} // namespace zeus
