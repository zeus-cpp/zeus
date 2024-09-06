#pragma once

#ifdef _WIN32

#include <string>
#include <memory>
#include <system_error>

#include <zeus/expected.hpp>

namespace zeus
{

enum class WinPowerRequestType
{
    DisplayRequried,
    SystemRequired,
    AwayModeRequired,
    ExecucutionRequired,
};

/**
 * @brief 电源请求
 *
 * RAII 类型的对象。持有此对象意味着持有电源请求。
 */
class WinPowerRequest
{
public:
    WinPowerRequest(WinPowerRequest const&)            = delete;
    WinPowerRequest& operator=(WinPowerRequest const&) = delete;

public:
    virtual ~WinPowerRequest() noexcept = 0;

    virtual WinPowerRequestType Type() const   = 0;
    virtual std::string         Reason() const = 0;

protected:
    WinPowerRequest() = default;
};

class WinPowerRequestFactory
{
public:
    /**
     * @brief 创建电源请求
     *
     * @param type 请求类型
     * @param reason 请求的原因，不可以为空。
     * @return 创建成功返回的对象，失败则返回空。
     */
    static zeus::expected<std::unique_ptr<WinPowerRequest>, std::error_code> Create(WinPowerRequestType type, const std::string& reason);
};

} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
