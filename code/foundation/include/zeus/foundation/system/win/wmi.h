#pragma once
#ifdef _WIN32
#include <memory>
#include <system_error>
#include <zeus/expected.hpp>
#include <nlohmann/json.hpp>

namespace zeus
{
struct WinWMIImpl;
class WinWMI
{
public:
    WinWMI();
    WinWMI(const WinWMI& other) = delete;
    WinWMI(WinWMI&& other) noexcept;
    WinWMI& operator=(const WinWMI&) = delete;
    WinWMI& operator=(WinWMI&& other) noexcept;
    ~WinWMI();
    zeus::expected<nlohmann::json, std::error_code> Query(const std::string& wql);
public:
    static zeus::expected<WinWMI, std::error_code> Create(const std::string& nameSpace = R"(ROOT\CIMV2)");
private:
    std::unique_ptr<WinWMIImpl> _impl;
};
}
#endif

#include "zeus/foundation/core/zeus_compatible.h"
