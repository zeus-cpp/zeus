#pragma once
#include <type_traits>

namespace zeus
{
#define ZEUS_ENUM_BIT_OPERATOR(EnumType)                                         \
    inline EnumType operator&(EnumType lhs, EnumType rhs)                        \
    {                                                                            \
        using T = std::underlying_type_t<EnumType>;                              \
        return static_cast<EnumType>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
    }                                                                            \
    inline EnumType operator|(EnumType lhs, EnumType rhs)                        \
    {                                                                            \
        using T = std::underlying_type_t<EnumType>;                              \
        return static_cast<EnumType>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
    }                                                                            \
    inline EnumType operator^(EnumType lhs, EnumType rhs)                        \
    {                                                                            \
        using T = std::underlying_type_t<EnumType>;                              \
        return static_cast<EnumType>(static_cast<T>(lhs) ^ static_cast<T>(rhs)); \
    }                                                                            \
    inline EnumType& operator&=(EnumType& lhs, EnumType rhs)                     \
    {                                                                            \
        lhs = lhs & rhs;                                                         \
        return lhs;                                                              \
    }                                                                            \
    inline EnumType& operator|=(EnumType& lhs, EnumType rhs)                     \
    {                                                                            \
        lhs = lhs | rhs;                                                         \
        return lhs;                                                              \
    }                                                                            \
    inline EnumType& operator^=(EnumType& lhs, EnumType rhs)                     \
    {                                                                            \
        lhs = lhs ^ rhs;                                                         \
        return lhs;                                                              \
    }

} // namespace zeus