#pragma once
#include <memory>
#include <string>
#include <system_error>
#include "zeus/expected.hpp"

namespace zeus
{
struct GlobalMutexImpl;
class GlobalMutex
{
public:
    GlobalMutex(const GlobalMutex&) = delete;
    GlobalMutex(GlobalMutex&& other) noexcept;
    GlobalMutex& operator=(const GlobalMutex&) = delete;
    GlobalMutex& operator=(GlobalMutex&& other) noexcept;
    ~GlobalMutex();
    std::string                           Name() const;
    zeus::expected<void, std::error_code> Lock();
    zeus::expected<void, std::error_code> Unlock();
    zeus::expected<bool, std::error_code> TryLock();
    //以下为兼容std::mutex的接口
    void                                  lock();
    void                                  unlock();
    bool                                  try_lock();
public:

    //名字需要符合操作系统的名称规范
    //windows下的名称规范和windows的文件名规范基本一致，如果想跨session使用需要拥有管理员权限再名称前面加上"Global\"
    //linux需遵循文件名规范，不要使用特殊字符
    static zeus::expected<GlobalMutex, std::error_code> OpenOrCreate(const std::string& name);
    static zeus::expected<GlobalMutex, std::error_code> Open(const std::string& name);
#ifdef __linux__
    static zeus::expected<void, std::error_code> Clear(const std::string& name);
#endif
protected:
    GlobalMutex();
private:
    std::unique_ptr<GlobalMutexImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
