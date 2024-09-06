#include "zeus/foundation/system/win/transaction.h"
#ifdef _WIN32
#include <cassert>
#include <Windows.h>
#include <ktmw32.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/resource/win/handle.h"

#pragma comment(lib, "ktmw32.lib")

namespace zeus
{
struct WinTransactionImpl
{
    WinHandle handle;
};

WinTransaction::WinTransaction() : _impl(std::make_unique<WinTransactionImpl>())
{
}

WinTransaction::~WinTransaction()
{
    if (_impl)
    {
        assert(_impl->handle);
        CommitTransaction(_impl->handle);
    }
}

WinTransaction::WinTransaction(WinTransaction&& other) noexcept : _impl(std::move(other._impl))
{
}

WinTransaction& WinTransaction::operator=(WinTransaction&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

WinTransaction::operator HANDLE() const noexcept
{
    return _impl->handle;
}

HANDLE WinTransaction::Handle() const noexcept
{
    return _impl->handle;
}

zeus::expected<void, std::error_code> WinTransaction::Commit()
{
    assert(_impl->handle);
    if (!CommitTransaction(_impl->handle))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

zeus::expected<void, std::error_code> WinTransaction::Rollback()
{
    assert(_impl->handle);
    if (!RollbackTransaction(_impl->handle))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

zeus::expected<WinTransaction, std::error_code> WinTransaction::Create(
    const std::string& name, const std::optional<std::chrono::milliseconds>& timeout
)
{
    auto   wname  = CharsetUtils::UTF8ToUnicode(name);
    HANDLE handle = CreateTransaction(nullptr, nullptr, 0, 0, 0, timeout.has_value() ? timeout->count() : 0, wname.data());
    if (INVALID_HANDLE_VALUE == handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WinTransaction transaction;
    transaction._impl->handle = handle;
    return transaction;
}

} // namespace zeus
#endif