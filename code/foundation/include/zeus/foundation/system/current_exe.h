#pragma once

#include <string>
#include <filesystem>
#include "zeus/foundation/system/process.h"
#include "zeus/foundation/string/version.h"

namespace zeus
{
namespace CurrentExe
{
//当前进程是否以服务的形式运行
bool         IsService();
//获取当前进程ID
Process::PID GetProcessId();

std::string GetUsername();

// 获取当前可执行文件的文件名（不包含后缀名）
std::string GetAppName();

// 获取当前可执行文件的完整路径
std::filesystem::path GetAppPath();
std::string           GetAppPathString();
//获取当前可执行文件的完整目录
std::filesystem::path GetAppDir();
std::string           GetAppDirString();
//获取当前执行代码所在模块(动态库、可执行文件)的完整路径
std::filesystem::path GetModulePath(void* address = nullptr);
std::string           GetModulePathString(void* address = nullptr);
//获取当前执行代码所在模块(动态库、可执行文件)的完整目录
std::filesystem::path GetModuleDir(void* address = nullptr);
std::string           GetModuleDirString(void* address = nullptr);

#ifdef _WIN32

std::filesystem::path GetProgramDataDir();
std::filesystem::path GetAppDataDir();

std::string   GetProgramDataDirString();
std::string   GetAppDataDirString();
//获取当前进程所在SessionID
size_t        GetProcessSessionId();
// 获取当前可执行文件的文件版本
zeus::Version GetFileVersion();
// 获取当前可执行文件的产品版本
zeus::Version GetProductVersion();
//获取当前执行代码所在模块(动态库、可执行文件)的文件版本
zeus::Version GetModuleFileVersion(void* address = nullptr);
//获取当前执行代码所在模块(动态库、可执行文件)的产品版本
zeus::Version GetModuleProductVersion(void* address = nullptr);

bool IsWow64();
#endif

#ifdef __linux__
uid_t       GetUserId();
gid_t       GetGroupId();
uid_t       GetEffectiveUserId();
gid_t       GetEffectiveGroupId();
bool        HasRootPermission();
std::string GetEffectiveUsername();
#endif
} // namespace CurrentExe
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
