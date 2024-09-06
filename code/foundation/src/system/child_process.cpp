#include "zeus/foundation/system/child_process.h"
#include "impl/child_process_impl.h"

namespace zeus
{
ChildProcessExecutor::ChildProcessExecutor() : _impl(std::make_unique<ChildProcessExecutorImpl>())
{
}

ChildProcessExecutor::~ChildProcessExecutor()
{
}

ChildProcessExecutor& ChildProcessExecutor::EnableRedirectStdout(bool redirect)
{
    _impl->redirectStdout = redirect;
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::EnableRedirectStderr(bool redirect)
{
    _impl->redirectStderr = redirect;
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::EnableRedirectStdin(bool redirect)
{
    _impl->redirectStdin = redirect;
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::SetEnvironment(std::string_view name, std::string_view value)
{
    _impl->environmentVariables[std::string(name)] = std::string(value);
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::SetWorkingDirectory(std::string_view path)
{
    _impl->workDir = std::string(path);
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::SetWorkingDirectory(const std::filesystem::path& path)
{
    _impl->workDir = path.u8string();
    return *this;
}

#ifdef _WIN32
ChildProcessExecutor& ChildProcessExecutor::EnableShow(bool show)
{
    _impl->show = show;
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::SetUseToken(HANDLE token, bool useEnv)
{
    _impl->token       = token;
    _impl->useTokenEnv = useEnv;
    return *this;
}

ChildProcessExecutor& ChildProcessExecutor::AddInheritedHandle(HANDLE handle)
{
    _impl->inheritedHandles.emplace_back(handle);
    return *this;
}
#endif

#ifdef __linux__
ChildProcessExecutor& ChildProcessExecutor::AddInheritedFd(int fd)
{
    _impl->inheritedFds.emplace_back(fd);
    return *this;
}
ChildProcessExecutor& ChildProcessExecutor::DisableStdout()
{
    _impl->disableStdout = true;
    return *this;
}
ChildProcessExecutor& ChildProcessExecutor::DisableStderr()
{
    _impl->disableStderr = true;
    return *this;
}
ChildProcessExecutor& ChildProcessExecutor::DisableStdin()
{
    _impl->disableStdin = true;
    return *this;
}
ChildProcessExecutor& ChildProcessExecutor::SetUID(uid_t uid)
{
    _impl->uid = uid;
    return *this;
}
ChildProcessExecutor& ChildProcessExecutor::SetGID(gid_t gid)
{
    _impl->gid = gid;
    return *this;
}
#endif
} // namespace zeus