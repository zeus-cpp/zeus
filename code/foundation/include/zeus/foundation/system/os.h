#pragma once

#include <string>
#include <chrono>
#include <zeus/foundation/string/version.h>

namespace zeus
{
namespace OS
{
std::string                           OsKernelName();
std::string                           OsDisplayName();
std::string                           OsProductName();
//Windows目前都一定有Major.Minor.Patch.Build版本号，linux的发行版中可能没有符合Major.Minor.Patch.Build格式的版本号
zeus::Version                         OsVersion();
//windows的版本号字符串是标准的Major.Minor.Patch.Build，linux的发行版中可能含有其他字符
std::string                           OsVersionString();
//Windows目前都一定有Major.Minor.Patch.Build版本号，linux的发行版中可能没有符合Major.Minor.Patch.Build格式的版本号
zeus::Version                         OsKernelVersion();
//windows的版本号字符串是标准的Major.Minor.Patch.Build，linux的发行版中可能含有其他字符
std::string                           OsKernelVersionString();
std::string                           OsArchitecture();
bool                                  IsOs64Bits();
std::string                           OsProductType();
//此名称的表示通常为系统名称、内核、位数、版本、发布号的综合。
std::string                           OsFullName();
std::chrono::system_clock::time_point OsBootTime();
std::chrono::steady_clock::duration   OsUpDuration();

bool IsUnix();
bool IsWindows();

#ifdef _WIN32
//操作系统本地化的产品名
std::string                           OsNativeProductName();
//只有win10开始 才有此属性
std::string                           OsDisplayVersion();
std::string                           OsProductId();
std::string                           OsMachineId();
//系统安装时间（当前时区）
std::chrono::system_clock::time_point OsInstallTime();
//是否是Windows服务器版本
bool                                  IsWinServer();
//版本判断依据版本号
bool                                  IsWin7();
bool                                  IsWin8();
bool                                  IsWin8_1();
bool                                  IsWin10();
bool                                  IsWin11();
bool                                  IsWin7OrGreater();
bool                                  IsWin8OrGreater();
bool                                  IsWin8_1OrGreater();
bool                                  IsWin10OrGreater();
bool                                  IsWin11OrGreater();

bool IsWinServer2008();
bool IsWinServer2008_R2();
bool IsWinServer2012();
bool IsWinServer2012_R2();
bool IsWinServer2016();
bool IsWinServer2019();
bool IsWinServer2022();
bool IsWinServer2008OrGreater();
bool IsWinServer2008_R2OrGreater();
bool IsWinServer2012OrGreater();
bool IsWinServer2012_R2OrGreater();
bool IsWinServer2016OrGreater();
bool IsWinServer2019OrGreater();
bool IsWinServer2022OrGreater();

/*
    Windows 10 Version
    Code     Version     Build
    TH1      1507        10240
    TH2      1511        10586
    RS1      1607        14393
    RS2      1703        15063
    RS3      1709        16299
    RS4      1803        17134
    RS5      1809        17763
    19H1     1903        18362
    19H2     1909        18363
    20H1     2004        19041
    20H2     20H2        19042
    21H1     21H1        19043
    21H2     21H2        19044
    */

/*
    Windows 11 Version
    Code     Version     Build
    21H2     21H2        22000
    22H2     22H2        22621
    23H2     23H2        22631
    */

bool IsWin10_TH1();
bool IsWin10_TH2();
bool IsWin10_RS1();
bool IsWin10_RS2();
bool IsWin10_RS3();
bool IsWin10_RS4();
bool IsWin10_RS5();
bool IsWin10_19H1();
bool IsWin10_19H2();
bool IsWin10_20H1();
bool IsWin10_20H2();
bool IsWin10_21H1();
bool IsWin10_21H2();

bool IsWin11_21H2();
bool IsWin11_22H2();
bool IsWin11_23H2();

bool IsWin10_TH1OrGreater();
bool IsWin10_TH2OrGreater();
bool IsWin10_RS1OrGreater();
bool IsWin10_RS2OrGreater();
bool IsWin10_RS3OrGreater();
bool IsWin10_RS4OrGreater();
bool IsWin10_RS5OrGreater();
bool IsWin10_19H1OrGreater();
bool IsWin10_19H2OrGreater();
bool IsWin10_20H1OrGreater();
bool IsWin10_20H2OrGreater();
bool IsWin10_21H1OrGreater();
bool IsWin10_21H2OrGreater();

bool IsWin11_21H2OrGreater();
bool IsWin11_22H2OrGreater();
bool IsWin11_23H2OrGreater();

#endif
} // namespace OS
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
