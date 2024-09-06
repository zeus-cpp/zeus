#pragma once
#include "zeus/foundation/system/child_process.h"
#include <list>
#include <optional>
#ifdef __linux__
#include <unistd.h>
#endif

namespace zeus
{
struct ChildProcessExecutorImpl
{
    std::map<std::string, std::string> environmentVariables;
    bool                               redirectStdout = false;
    bool                               redirectStderr = false;
    bool                               redirectStdin  = false;
    std::string                        workDir;
#ifdef _WIN32
    bool              show = false;
    std::list<HANDLE> inheritedHandles;
    HANDLE            token       = nullptr;
    bool              useTokenEnv = false;
#endif
#ifdef __linux__
    std::list<int>       inheritedFds;
    std::optional<uid_t> uid;
    std::optional<gid_t> gid;
    bool                 disableStdout = false;
    bool                 disableStderr = false;
    bool                 disableStdin  = false;
#endif
};
}