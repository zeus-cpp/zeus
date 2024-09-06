#pragma once
#ifdef _WIN32
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <filesystem>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{
struct WinDigtalSignImpl;
class WinDigtalSign
{
public:
    WinDigtalSign(const WinDigtalSign&) = delete;
    WinDigtalSign(WinDigtalSign&& other) noexcept;
    WinDigtalSign& operator=(const WinDigtalSign&) = delete;
    WinDigtalSign& operator=(WinDigtalSign&& other) noexcept;
    ~WinDigtalSign();
    //签名者名称
    std::optional<std::string> GetSubjectName() const;
    //序列号
    std::optional<std::string> GetSerialNumber() const;
    //颁发者名称
    std::optional<std::string> GetIssuerName() const;
    friend class WinFileDigtalSign;
protected:
    WinDigtalSign();
private:
    std::shared_ptr<WinDigtalSignImpl> _impl;
};

struct WinFileDigtalSignImpl;
class WinFileDigtalSign
{
public:
    ~WinFileDigtalSign();
    WinFileDigtalSign(const WinFileDigtalSign&) = delete;
    WinFileDigtalSign(WinFileDigtalSign&& other) noexcept;
    WinFileDigtalSign& operator=(const WinFileDigtalSign&) = delete;
    WinFileDigtalSign& operator=(WinFileDigtalSign&& other) noexcept;

    const std::vector<WinDigtalSign>& GetDigtalSigns() const;
public:
    //nested 表示是否解析嵌套证书，windows下的多签名是用嵌套证书实现的
    static zeus::expected<WinFileDigtalSign, std::error_code> Load(const std::filesystem::path& path, bool nested = false);
protected:
    WinFileDigtalSign();
private:
    std::unique_ptr<WinFileDigtalSignImpl> _impl;
};
} // namespace zeus

#endif //_WIN32

#include "zeus/foundation/core/zeus_compatible.h"
