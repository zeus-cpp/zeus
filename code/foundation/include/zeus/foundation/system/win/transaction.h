#pragma once
#ifdef _WIN32
#include <memory>
#include <string>
#include <chrono>
#include <optional>
#include <system_error>
#include <zeus/expected.hpp>
#include "zeus/foundation/core/win/win_windef.h"

namespace zeus
{
struct WinTransactionImpl;
class WinTransaction
{
public:
    ~WinTransaction();
    WinTransaction(const WinTransaction&)            = delete;
    WinTransaction& operator=(const WinTransaction&) = delete;
    WinTransaction(WinTransaction&& other) noexcept;
    WinTransaction& operator=(WinTransaction&& other) noexcept;
    operator HANDLE() const noexcept;

    HANDLE                                Handle() const noexcept;
    zeus::expected<void, std::error_code> Commit();
    zeus::expected<void, std::error_code> Rollback();
public:
    static zeus::expected<WinTransaction, std::error_code> Create(
        const std::string& name, const std::optional<std::chrono::milliseconds>& timeout = std::nullopt
    );
protected:
    WinTransaction();
private:
    std::unique_ptr<WinTransactionImpl> _impl;
};
} // namespace zeus
#endif
#include "zeus/foundation/core/zeus_compatible.h"
