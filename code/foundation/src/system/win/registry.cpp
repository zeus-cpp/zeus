#ifdef _WIN32
#include <cassert>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/byte/byte_utils.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/system/win/registry.h"

namespace zeus
{

namespace
{

HKEY ParseRootKey(const std::string& key)
{
    if (StartWith(key, "HKEY_CLASSES_ROOT"))
    {
        return HKEY_CLASSES_ROOT;
    }
#if defined(HKEY_CURRENT_CONFIG)
    else if (StartWith(key, "HKEY_CURRENT_CONFIG"))
    {
        return HKEY_CURRENT_CONFIG;
    }
#endif
    else if (StartWith(key, "HKEY_CURRENT_USER"))
    {
        return HKEY_CURRENT_USER;
    }
    else if (StartWith(key, "HKEY_LOCAL_MACHINE"))
    {
        return HKEY_LOCAL_MACHINE;
    }
    else if (StartWith(key, "HKEY_USERS"))
    {
        return HKEY_USERS;
    }
#if defined(HKEY_PERFORMANCE_DATA)
    else if (StartWith(key, "HKEY_PERFORMANCE_DATA"))
    {
        return HKEY_PERFORMANCE_DATA;
    }
#endif
    else
    {
        return nullptr;
    }
}

DWORD WowSamDesired(WinRegistry::ViewType viewType)
{
    switch (viewType)
    {
    case WinRegistry::ViewType::WIN_KEY_32:
        return KEY_WOW64_32KEY;
    case WinRegistry::ViewType::WIN_KEY_64:
        return KEY_WOW64_64KEY;
    default:
        return 0;
    }
}
} // namespace

struct WinRegistryImpl
{
    HKEY                  hKey     = nullptr;
    WinRegistry::ViewType viewType = WinRegistry::ViewType ::AUTO;
};

WinRegistry::WinRegistry() : _impl(std::make_unique<WinRegistryImpl>())
{
}

WinRegistry::WinRegistry(WinRegistry&& other) noexcept : _impl(std::move(other._impl))
{
}

WinRegistry& WinRegistry::operator=(WinRegistry&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

WinRegistry::~WinRegistry()
{
    if (_impl)
    {
        if (_impl->hKey)
        {
            RegCloseKey(_impl->hKey);
            _impl->hKey = nullptr;
        }
    }
}

WinRegistry::operator HKEY() const noexcept
{
    assert(_impl);
    return _impl->hKey;
}

HKEY WinRegistry::Key() const noexcept
{
    assert(_impl);
    return _impl->hKey;
}

zeus::expected<WinRegistry::ValueType, std::error_code> WinRegistry::GetType(const std::string& name) const
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    DWORD        type  = 0;
    if (const auto result = RegQueryValueExW(_impl->hKey, wName.c_str(), nullptr, &type, nullptr, nullptr); ERROR_SUCCESS == result)
    {
        return static_cast<ValueType>(type);
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<uint32_t, std::error_code> WinRegistry::GetDwordValue(const std::string& name) const
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    uint32_t     value = 0;
    DWORD        size  = sizeof(value);
    if (const auto result = RegQueryValueExW(_impl->hKey, wName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
        ERROR_SUCCESS == result)
    {
        return value;
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<uint64_t, std::error_code> WinRegistry::GetQwordValue(const std::string& name) const
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    uint64_t     value = 0;
    DWORD        size  = sizeof(value);
    if (const auto result = RegQueryValueExW(_impl->hKey, wName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
        ERROR_SUCCESS == result)
    {
        return value;
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<std::string, std::error_code> WinRegistry::GetStringValue(const std::string& name) const
{
    auto data = GetDataValue(name);
    if (data)
    {
        return CharsetUtils::UnicodeToUTF8(reinterpret_cast<const wchar_t*>(data->data()));
    }
    return zeus::unexpected(data.error());
}
zeus::expected<std::string, std::error_code> WinRegistry::GetStringExpandValue(const std::string& name) const
{
    auto data = GetDataValue(name);
    if (data)
    {
        for (;;)
        {
            const DWORD expandCapacity = ExpandEnvironmentStringsW(reinterpret_cast<const wchar_t*>(data->data()), nullptr, 0);
            if (!expandCapacity)
            {
                return zeus::unexpected(GetLastSystemError());
            }
            auto       expandBuffer = std::make_unique<wchar_t[]>(expandCapacity);
            const auto expandSize   = ExpandEnvironmentStringsW(reinterpret_cast<const wchar_t*>(data->data()), expandBuffer.get(), expandCapacity);
            if (!expandSize)
            {
                return zeus::unexpected(GetLastSystemError());
            }
            if (expandSize == expandCapacity)
            {
                return CharsetUtils::UnicodeToUTF8(expandBuffer.get());
            }
        }
    }
    return zeus::unexpected(data.error());
}

zeus::expected<std::vector<std::string>, std::error_code> WinRegistry::GetMultiStringValue(const std::string& name) const
{
    auto data = GetDataValue(name);
    if (data)
    {
        auto                     wstrings = SplitMultiWString(data->data(), data->size());
        std::vector<std::string> result;
        result.reserve(wstrings.size());
        for (const auto& wstring : wstrings)
        {
            result.emplace_back(CharsetUtils::UnicodeToUTF8(wstring));
        }
        return std::move(result);
    }
    return zeus::unexpected(data.error());
}

zeus::expected<std::vector<uint8_t>, std::error_code> WinRegistry::GetBinaryValue(const std::string& name) const
{
    return GetDataValue(name);
}

zeus::expected<std::vector<uint8_t>, std::error_code> WinRegistry::GetDataValue(const std::string& name) const
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    DWORD        size  = 0;
    DWORD        type  = 0;
    if (auto result = RegQueryValueExW(_impl->hKey, wName.c_str(), nullptr, &type, nullptr, &size); ERROR_SUCCESS == result)
    {
        if (size)
        {
            bool                 isString = REG_SZ == type || REG_MULTI_SZ == type || REG_EXPAND_SZ == type;
            std::vector<uint8_t> buffer;
            if (isString)
            {
                buffer.resize(size + sizeof(wchar_t) * 2);
                std::memset(buffer.data(), 0, buffer.size());
            }
            else
            {
                buffer.resize(size);
            }
            result = RegQueryValueExW(_impl->hKey, wName.c_str(), nullptr, nullptr, buffer.data(), &size);
            if (ERROR_SUCCESS == result)
            {
                return std::move(buffer);
            }
            else
            {
                return zeus::unexpected(static_cast<SystemError>(result));
            }
        }
        return {};
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<std::vector<std::string>, std::error_code> WinRegistry::GetSubKeys() const
{
    assert(_impl->hKey);
    std::vector<std::string> subkeys;
    DWORD                    maxKeyLength = 0;
    if (const auto result =
            RegQueryInfoKeyW(_impl->hKey, nullptr, nullptr, nullptr, nullptr, &maxKeyLength, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        ERROR_SUCCESS != result)
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
    auto keyBuffer = std::make_unique<wchar_t[]>(maxKeyLength + 1);
    for (DWORD index = 0;; ++index)
    {
        DWORD      nameLength = maxKeyLength + 1;
        auto const result     = RegEnumKeyExW(_impl->hKey, index, keyBuffer.get(), &nameLength, nullptr, nullptr, nullptr, nullptr);
        if (ERROR_SUCCESS == result)
        {
            if (nameLength)
            {
                subkeys.emplace_back(CharsetUtils::UnicodeToUTF8(keyBuffer.get()));
            }
        }
        else if (ERROR_NO_MORE_ITEMS == result)
        {
            break;
        }
        else
        {
            return zeus::unexpected(static_cast<SystemError>(result));
        }
    }
    return std::move(subkeys);
}

zeus::expected<std::vector<std::string>, std::error_code> WinRegistry::GetValueNames() const
{
    assert(_impl->hKey);
    std::vector<std::string> valueNames;
    DWORD                    maxNameLength = 0;
    if (const auto result =
            RegQueryInfoKeyW(_impl->hKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &maxNameLength, nullptr, nullptr, nullptr);
        ERROR_SUCCESS != result)
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
    auto nameBuffer = std::make_unique<wchar_t[]>(maxNameLength + 1);
    for (DWORD index = 0;; ++index)
    {
        DWORD      nameLength = maxNameLength + 1;
        auto const result     = RegEnumValueW(_impl->hKey, index, nameBuffer.get(), &nameLength, nullptr, nullptr, nullptr, nullptr);
        if (ERROR_SUCCESS == result)
        {
            if (nameLength)
            {
                valueNames.emplace_back(CharsetUtils::UnicodeToUTF8(nameBuffer.get()));
            }
        }
        else if (ERROR_NO_MORE_ITEMS == result)
        {
            break;
        }
        else
        {
            return zeus::unexpected(static_cast<SystemError>(result));
        }
    }
    return std::move(valueNames);
}

zeus::expected<bool, std::error_code> WinRegistry::ExistsValue(const std::string& name) const
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    if (const auto result = RegQueryValueExW(_impl->hKey, wName.c_str(), nullptr, nullptr, nullptr, nullptr); ERROR_SUCCESS == result)
    {
        return true;
    }
    else if (ERROR_FILE_NOT_FOUND == result)
    {
        return false;
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<void, std::error_code> WinRegistry::DeleteValue(const std::string& name)
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    if (const auto result = RegDeleteValueW(_impl->hKey, wName.c_str()); ERROR_SUCCESS == result)
    {
        return {};
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<void, std::error_code> WinRegistry::SetDwordValue(const std::string& name, uint32_t value)
{
    return SetDataValue(name, ValueType::kDWORD, &value, sizeof(value));
}

zeus::expected<void, std::error_code> WinRegistry::SetQwordValue(const std::string& name, uint64_t value)
{
    return SetDataValue(name, ValueType::kQWORD, &value, sizeof(value));
}

zeus::expected<void, std::error_code> WinRegistry::SetStringValue(const std::string& name, const std::string& value)
{
    std::wstring wValue = CharsetUtils::UTF8ToUnicode(value);
    return SetDataValue(name, ValueType::kSTRING, wValue.data(), (wValue.size() + 1) * sizeof(wchar_t));
}
zeus::expected<void, std::error_code> WinRegistry::SetStringExpandValue(const std::string& name, const std::string& value)
{
    std::wstring wValue = CharsetUtils::UTF8ToUnicode(value);
    return SetDataValue(name, ValueType::kSTRING_EXPAND, wValue.data(), (wValue.size() + 1) * sizeof(wchar_t));
}
zeus::expected<void, std::error_code> WinRegistry::SetMultiStringValue(const std::string& name, const std::vector<std::string>& value)
{
    std::vector<std::wstring> wValue;
    wValue.reserve(value.size());
    size_t bufferSize = 0;
    for (const auto& element : value)
    {
        wValue.emplace_back(CharsetUtils::UTF8ToUnicode(element));
        bufferSize += (wValue.back().size() + 1) * sizeof(wchar_t);
    }
    bufferSize += sizeof(wchar_t);
    std::vector<uint8_t> buffer;
    buffer.reserve(bufferSize);
    for (const auto& element : wValue)
    {
        buffer.insert(
            buffer.end(), reinterpret_cast<const uint8_t*>(element.c_str()),
            reinterpret_cast<const uint8_t*>(element.c_str() + (element.size() + 1) * sizeof(wchar_t))
        );
    }
    buffer.emplace_back(0);
    buffer.emplace_back(0);
    return SetDataValue(name, ValueType::kMULTI_STRING, buffer.data(), buffer.size());
}
zeus::expected<void, std::error_code> WinRegistry::SetBinaryValue(const std::string& name, const void* data, size_t size)
{
    return SetDataValue(name, ValueType::kBINARY, data, size);
}

zeus::expected<void, std::error_code> WinRegistry::SetDataValue(const std::string& name, ValueType type, const void* value, size_t size)
{
    assert(_impl->hKey);
    std::wstring wName = CharsetUtils::UTF8ToUnicode(name);
    if (const auto result = RegSetValueExW(_impl->hKey, wName.c_str(), 0, static_cast<DWORD>(type), reinterpret_cast<const BYTE*>(value), size);
        ERROR_SUCCESS == result)
    {
        return {};
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<void, std::error_code> WinRegistry::Close()
{
    if (_impl->hKey)
    {
        const auto result = RegCloseKey(_impl->hKey);
        _impl->hKey       = nullptr;
        if (ERROR_SUCCESS == result)
        {
            return {};
        }
        else
        {
            return zeus::unexpected(static_cast<SystemError>(result));
        }
    }
    return {};
}

zeus::expected<void, std::error_code> WinRegistry::Flush()
{
    assert(_impl->hKey);
    const auto result = RegFlushKey(_impl->hKey);
    if (ERROR_SUCCESS == result)
    {
        return {};
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<void, std::error_code> WinRegistry::DeleteKey(const std::string& key)
{
    auto const  pos = key.find(R"(\)");
    std::string subkey;
    if (pos != std::string::npos)
    {
        subkey = key.substr(pos + 1);
    }
    auto* rootKey = ParseRootKey(key);
    return DeleteKey(rootKey, subkey);
}

zeus::expected<void, std::error_code> WinRegistry::DeleteKey(HKEY rootKey, const std::string& subKey)
{
    std::wstring wSubkey = CharsetUtils::UTF8ToUnicode(subKey);
    if (auto const result = RegDeleteTreeW(rootKey, wSubkey.c_str()); ERROR_SUCCESS == result)
    {
        return {};
    }
    else if (ERROR_FILE_NOT_FOUND == result)
    {
        return {};
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<bool, std::error_code> WinRegistry::ExistsKey(const std::string& key, ViewType viewType)
{
    auto const  pos = key.find(R"(\)");
    std::string subkey;
    if (pos != std::string::npos)
    {
        subkey = key.substr(pos + 1);
    }
    auto* rootKey = ParseRootKey(key);
    return ExistsKey(rootKey, subkey, viewType);
}

zeus::expected<bool, std::error_code> WinRegistry::ExistsKey(HKEY rootKey, const std::string& subKey, ViewType viewType)
{
    HKEY         hKey    = nullptr;
    std::wstring wSubkey = CharsetUtils::UTF8ToUnicode(subKey);
    auto const   result  = RegOpenKeyExW(rootKey, wSubkey.c_str(), 0, KEY_QUERY_VALUE | WowSamDesired(viewType), &hKey);
    if (ERROR_SUCCESS == result)
    {
        RegCloseKey(hKey);
        return true;
    }
    else if (ERROR_FILE_NOT_FOUND == result)
    {
        return false;
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::OpenKey(const std::string& key, bool read, bool write, ViewType viewType)
{
    auto const  pos = key.find(R"(\)");
    std::string subkey;
    if (pos != std::string::npos)
    {
        subkey = key.substr(pos + 1);
    }
    auto* rootKey = ParseRootKey(key);
    return OpenKey(rootKey, subkey, read, write, viewType);
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::OpenKey(HKEY rootKey, const std::string& subKey, bool read, bool write, ViewType viewType)
{
    HKEY         hKey    = nullptr;
    std::wstring wSubkey = CharsetUtils::UTF8ToUnicode(subKey);
    auto const result = RegOpenKeyExW(rootKey, wSubkey.c_str(), 0, (read ? KEY_READ : 0) | (write ? KEY_WRITE : 0) | WowSamDesired(viewType), &hKey);
    if (ERROR_SUCCESS == result)
    {
        WinRegistry registry;
        registry._impl->hKey = hKey;
        return std::move(registry);
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::CreateKey(
    const std::string& key, bool read, bool write, ViewType viewType, bool persistence, bool backupRestore
)
{
    auto const  pos = key.find(R"(\)");
    std::string subkey;
    if (pos != std::string::npos)
    {
        subkey = key.substr(pos + 1);
    }
    auto* rootKey = ParseRootKey(key);
    return CreateKey(rootKey, subkey, read, write, viewType, persistence, backupRestore);
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::CreateKey(
    HKEY rootKey, const std::string& subKey, bool read, bool write, ViewType viewType, bool persistence, bool backupRestore
)
{
    HKEY         hKey    = nullptr;
    std::wstring wSubkey = CharsetUtils::UTF8ToUnicode(subKey);
    auto const   result  = RegCreateKeyExW(
        rootKey, wSubkey.c_str(), 0, nullptr,
        (persistence ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE) | (backupRestore ? REG_OPTION_BACKUP_RESTORE : 0),
        (read ? KEY_READ : 0) | (write ? KEY_WRITE : 0) | WowSamDesired(viewType), nullptr, &hKey, nullptr
    );
    if (ERROR_SUCCESS == result)
    {
        WinRegistry registry;
        registry._impl->hKey = hKey;
        return std::move(registry);
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::OpenTransactedKey(
    HANDLE transaction, const std::string& key, bool read, bool write, ViewType viewType
)
{
    auto const  pos = key.find(R"(\)");
    std::string subkey;
    if (pos != std::string::npos)
    {
        subkey = key.substr(pos + 1);
    }
    auto* rootKey = ParseRootKey(key);
    return OpenTransactedKey(transaction, rootKey, subkey, read, write, viewType);
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::OpenTransactedKey(
    HANDLE transaction, HKEY rootKey, const std::string& subKey, bool read, bool write, ViewType viewType
)
{
    HKEY         hKey    = nullptr;
    std::wstring wSubkey = CharsetUtils::UTF8ToUnicode(subKey);
    auto const   result  = RegOpenKeyTransactedW(
        rootKey, wSubkey.c_str(), 0, (read ? KEY_READ : 0) | (write ? KEY_WRITE : 0) | WowSamDesired(viewType), &hKey, transaction, nullptr
    );
    if (ERROR_SUCCESS == result)
    {
        WinRegistry registry;
        registry._impl->hKey = hKey;
        return std::move(registry);
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::CreateTransactedKey(
    HANDLE transaction, const std::string& key, bool read, bool write, ViewType viewType, bool persistence, bool backupRestore
)
{
    auto const  pos = key.find(R"(\)");
    std::string subkey;
    if (pos != std::string::npos)
    {
        subkey = key.substr(pos + 1);
    }
    auto* rootKey = ParseRootKey(key);
    return CreateTransactedKey(transaction, rootKey, subkey, read, write, viewType, persistence, backupRestore);
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::CreateTransactedKey(
    HANDLE transaction, HKEY rootKey, const std::string& subKey, bool read, bool write, ViewType viewType, bool persistence, bool backupRestore
)
{
    HKEY         hKey    = nullptr;
    std::wstring wSubkey = CharsetUtils::UTF8ToUnicode(subKey);
    auto const   result  = RegCreateKeyTransactedW(
        rootKey, wSubkey.c_str(), 0, nullptr,
        (persistence ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE) || (backupRestore ? REG_OPTION_BACKUP_RESTORE : 0),
        (read ? KEY_READ : 0) | (write ? KEY_WRITE : 0) | WowSamDesired(viewType), nullptr, &hKey, nullptr, transaction, nullptr
    );
    if (ERROR_SUCCESS == result)
    {
        WinRegistry registry;
        registry._impl->hKey = hKey;
        return std::move(registry);
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

zeus::expected<WinRegistry, std::error_code> WinRegistry::OpenCurrentUserKey(bool read, bool write)
{
    HKEY       hKey   = nullptr;
    auto const result = RegOpenCurrentUser((read ? KEY_READ : 0) | (write ? KEY_WRITE : 0), &hKey);
    if (ERROR_SUCCESS == result)
    {
        WinRegistry registry;
        registry._impl->hKey = hKey;
        return std::move(registry);
    }
    else
    {
        return zeus::unexpected(static_cast<SystemError>(result));
    }
}

} // namespace zeus

#endif
