#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>
#include <system_error>
#ifdef __linux__
#include <unistd.h>
#endif
#include <zeus/expected.hpp>
#include "zeus/foundation/core/win/win_windef.h"
#include "zeus/foundation/system/process.h"

namespace zeus
{
class WinToken;
namespace Environment
{
size_t                   ProcessorCount();
size_t                   ProcessorCoreCount();
std::vector<std::string> GetActiveUsers();
std::filesystem::path    GetHome();
std::string              GetHomeString();
std::string              GetMacId();
std::string              GetMachineName();
std::filesystem::path    GetDesktopPath();
std::string              GetDesktopPathString();

std::filesystem::path GetGlobalAppDataPath();
std::string           GetGlobalAppDataPathString();

#ifdef _WIN32
std::filesystem::path GetWindowsFolderPath(const GUID& rfid);
std::filesystem::path GetWindowsFolderPath(const GUID& rfid, const WinToken& token);
std::filesystem::path GetPublicDesktopPath();

std::string GetWindowsFolderPathString(const GUID& rfid);
std::string GetWindowsFolderPathString(const GUID& rfid, const WinToken& token);
std::string GetPublicDesktopPathString();
#endif

#ifdef __linux__
zeus::expected<uid_t, std::error_code>                 GetUserId(const std::string& username);
zeus::expected<std::string, std::error_code>           GetUsername(uid_t uid);
zeus::expected<std::filesystem::path, std::error_code> GetUserHome(uid_t uid);
zeus::expected<std::filesystem::path, std::error_code> GetUserHome(const std::string& username);
std::optional<std::map<std::string, std::string>>      GetActiveX11Environment();
bool                                                   IsService(Process::PID pid);
#endif

} // namespace Environment
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
