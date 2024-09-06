#include "zeus/foundation/system/win/sid.h"
#ifdef _WIN32
#include <cassert>
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable : 4005)
#include <ntstatus.h>
#pragma warning(pop)
#include <ntsecapi.h>
#include <sddl.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/string/charset_utils.h"

namespace zeus
{
struct WinAccountImpl
{
    std::string username;
    std::string domain;
};

WinAccount::WinAccount(const std::string &domain, const std::string &username) : _impl(std::make_unique<WinAccountImpl>())
{
    _impl->domain   = domain;
    _impl->username = username;
}

WinAccount::~WinAccount()
{
}

WinAccount::WinAccount(const WinAccount &other) noexcept : _impl(std::make_unique<WinAccountImpl>(*other._impl))
{
}

WinAccount::WinAccount(WinAccount &&other) noexcept : _impl(std::move(other._impl))
{
}

WinAccount &WinAccount::operator=(const WinAccount &other) noexcept
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

WinAccount &WinAccount::operator=(WinAccount &&other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

bool WinAccount::operator==(const WinAccount &other) const noexcept
{
    return _impl->domain == other._impl->domain && _impl->username == other._impl->username;
}

bool WinAccount::operator!=(const WinAccount &other) const noexcept
{
    return _impl->domain != other._impl->domain || _impl->username != other._impl->username;
}

std::string WinAccount::Username() const
{
    return _impl->username;
}

std::string WinAccount::Domain() const
{
    return _impl->domain;
}

std::string WinAccount::ToString() const
{
    return _impl->domain + R"(\)" + _impl->username;
}

struct WinSIDImpl
{
    std::vector<uint8_t> sidData;
};

WinSID::WinSID(size_t sidLength) : _impl(std::make_unique<WinSIDImpl>())
{
    _impl->sidData.resize(sidLength);
}

WinSID::~WinSID()
{
}

WinSID::WinSID(WinSID &&other) noexcept : _impl(std::move(other._impl))
{
}

WinSID &WinSID::operator=(WinSID &&other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

WinSID::operator PSID() const noexcept
{
    return static_cast<PSID>(_impl->sidData.data());
}

bool WinSID::operator==(const WinSID &other) const noexcept
{
    return IsSIDEqual(*this, other);
}

bool WinSID::operator!=(const WinSID &other) const noexcept
{
    return !IsSIDEqual(*this, other);
}

bool WinSID::operator==(PSID other) const noexcept
{
    return IsSIDEqual(*this, other);
}

bool WinSID::operator!=(PSID other) const noexcept
{
    return !IsSIDEqual(*this, other);
}

size_t WinSID::Length() const noexcept
{
    return _impl->sidData.size();
}

PSID WinSID::Psid() const noexcept
{
    return static_cast<PSID>(_impl->sidData.data());
}

zeus::expected<WinAccount, std::error_code> WinSID::GetAccount() const
{
    return GetSIDAccount(*this);
}

zeus::expected<std::string, std::error_code> WinSID::GetString() const
{
    return GetSIDString(*this);
}

zeus::expected<WinAccount, std::error_code> WinSID::GetSIDAccount(PSID sid)
{
    DWORD nameLength   = 0;
    DWORD domainLength = 0;
    LookupAccountSidW(nullptr, sid, nullptr, &nameLength, nullptr, &domainLength, nullptr);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        auto         nameBuffer   = std::make_unique<wchar_t[]>(nameLength);
        auto         domainBuffer = std::make_unique<wchar_t[]>(domainLength);
        SID_NAME_USE use;
        if (LookupAccountSidW(nullptr, sid, nameBuffer.get(), &nameLength, domainBuffer.get(), &domainLength, &use))
        {
            return WinAccount(CharsetUtils::UnicodeToUTF8(domainBuffer.get()), CharsetUtils::UnicodeToUTF8(nameBuffer.get()));
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(TranslateToSystemError(error));
    }
}

zeus::expected<std::string, std::error_code> WinSID::GetSIDString(PSID sid)
{
    char       *buffer = nullptr;
    AutoRelease raii(
        [&buffer]()
        {
            if (buffer)
            {
                LocalFree(buffer);
            }
        }
    );
    if (ConvertSidToStringSidA(sid, &buffer))
    {
        return std::string(buffer);
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<WinSID, std::error_code> WinSID::CopySID(PSID sid)
{
    DWORD sidLength = GetLengthSid(sid);
    if (sidLength)
    {
        WinSID winSid(sidLength);
        if (CopySid(sidLength, winSid, sid))
        {
            return std::move(winSid);
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<WinSID, std::error_code> WinSID::GetAccountSID(const std::string &username)
{
    DWORD sidLength    = 0;
    DWORD domainLength = 0;
    LookupAccountNameA(nullptr, username.c_str(), nullptr, &sidLength, nullptr, &domainLength, nullptr);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        WinSID       winSid(sidLength);
        auto         domainBuffer = std::make_unique<char[]>(domainLength);
        SID_NAME_USE use          = {};
        if (LookupAccountNameA(nullptr, username.c_str(), winSid, &sidLength, domainBuffer.get(), &domainLength, &use))
        {
            return std::move(winSid);
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(TranslateToSystemError(error));
    }
}
zeus::expected<WinSID, std::error_code> WinSID::GetWellknowSID(WELL_KNOWN_SID_TYPE type)
{
    DWORD sidLength = 0;
    CreateWellKnownSid(type, nullptr, nullptr, &sidLength);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        WinSID winSid(sidLength);
        if (CreateWellKnownSid(type, nullptr, winSid, &sidLength))
        {
            return std::move(winSid);
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(TranslateToSystemError(error));
    }
}
bool WinSID::IsSIDEqual(PSID sid1, PSID sid2) noexcept
{
    const SID *first  = static_cast<SID *>(sid1);
    const SID *second = static_cast<SID *>(sid2);
    if ((first && !second) || (!first && second))
    {
        return false;
    }
    if (!first && !second)
    {
        return true;
    }
    assert(first && second);
    if (first->SubAuthorityCount == second->SubAuthorityCount)
    {
        const auto SidLength = FIELD_OFFSET(SID, SubAuthority[first->SubAuthorityCount]);
        return 0 == std::memcmp(sid1, sid2, SidLength);
    }
    else
    {
        return false;
    }
}

} // namespace zeus
#endif
