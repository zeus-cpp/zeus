#pragma once
#include <memory>
#include <string>
#include <chrono>
#include <utility>
#include <system_error>
#include <zeus/expected.hpp>
#include "zeus/foundation/system/process.h"

namespace zeus
{
struct GlobalEventImpl;
class GlobalEvent
{
public:
    GlobalEvent(const GlobalEvent&) = delete;
    GlobalEvent(GlobalEvent&& other) noexcept;
    GlobalEvent& operator=(const GlobalEvent&) = delete;
    GlobalEvent& operator=(GlobalEvent&& other) noexcept;
    ~GlobalEvent();

    std::string                           Name() const;
    zeus::expected<void, std::error_code> Set();
    zeus::expected<void, std::error_code> Reset();
    zeus::expected<void, std::error_code> Wait();
    zeus::expected<bool, std::error_code> WaitTimeout(const std::chrono::steady_clock::duration& duration);
#ifdef __linux__
    zeus::expected<zeus::Process::PID, std::error_code> AliveCreator();
#endif
public:
    //名字需要符合操作系统的名称规范
    //windows下的名称规范和windows的文件名规范基本一致，如果想跨session使用需要拥有管理员权限再名称前面加上"Global\"
    //linux需遵循文件名规范，不要使用特殊字符
    //std::pair<event,exist>
    static zeus::expected<GlobalEvent, std::error_code> OpenOrCreate(const std::string& name, bool manual);
    static zeus::expected<GlobalEvent, std::error_code> Open(const std::string& name);
#ifdef __linux__
    static zeus::expected<void, std::error_code> Clear(const std::string& name);
#endif
protected:
    GlobalEvent();
private:
    std::unique_ptr<GlobalEventImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
