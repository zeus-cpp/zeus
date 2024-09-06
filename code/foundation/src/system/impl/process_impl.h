#pragma once
#include <string>
#include <filesystem>
#include <limits>
#include <mutex>
#include <chrono>
#include "zeus/foundation/system/process.h"
#include "zeus/foundation/system/win/session.h"

namespace zeus
{
//此类的成员函数无法保证线程安全
struct ProcessImpl
{
    Process::PID                          pid = std::numeric_limits<Process::PID>::max();
    std::string                           name;
    Process::PID                          ppid = std::numeric_limits<Process::PID>::max();
    std::filesystem::path                 exePath;
    std::string                           exePathString;
    std::chrono::system_clock::time_point createTime;
    uint64_t                              createTimeTick = 0;
#ifdef _WIN32
    WinSession::SessionId sessionId = 0xFFFFFFFF;
#endif
#ifdef __linux__
    uid_t userId           = 0;
    gid_t groupId          = 0;
    uid_t effectiveUserId  = 0;
    gid_t effectiveGroupId = 0;
#endif
};
} // namespace zeus
