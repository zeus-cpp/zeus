#pragma once
#include <memory>
#include <string>
#include <optional>
#ifdef _WIN32
#include <Guiddef.h>
#endif
namespace zeus
{
struct UuidImpl;
class Uuid
{
public:
    Uuid();
    ~Uuid();
    Uuid(const Uuid& other);
    Uuid(Uuid&& other) noexcept;
    Uuid&       operator=(const Uuid& other);
    Uuid&       operator=(Uuid&& other) noexcept;
    static Uuid GenerateRandom();
    std::string toString(const std::string& split = "-", bool up = true) const;
#ifdef _WIN32
    //此方法是windows标准的字符串转换规则，默认会用{}包含，以-分割，和Uuid对象直接转化string不相等，如果要判断相等，请判断Uuid对象相等
    std::string        toWindowsString() const;
    //此方法是windows标准的字符串转换规则，默认会用{}包含，以-分割，和Uuid对象直接转化string不相等，如果要判断相等，请判断Uuid对象相等
    static std::string toString(const GUID& guid);
    static Uuid        FromGuid(const GUID& guid);
    GUID               toGuid() const;
#endif
    static std::optional<Uuid> FromWindowsString(const std::string& guid);

    friend bool operator==(Uuid const& lhs, Uuid const& rhs) noexcept;
    friend bool operator<(Uuid const& lhs, Uuid const& rhs) noexcept;
private:
    std::unique_ptr<UuidImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
