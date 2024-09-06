#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <system_error>
#include "zeus/expected.hpp"
#ifdef _WIN32
#include "zeus/foundation/core/win/win_windef.h"
#endif
#include "zeus/foundation/ipc/memory_mapping.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/resource/win/handle.h"

namespace zeus
{
struct SharedMemoryImpl;
class SharedMemory
{
public:

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&& other) noexcept;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory& operator=(SharedMemory&& other) noexcept;
    ~SharedMemory();
    std::string                                    Name() const;
    uint64_t                                       Size() const;
    zeus::expected<MemoryMapping, std::error_code> Map(uint64_t size, uint64_t offset = 0, bool readOnly = false);
#ifdef _WIN32
    HANDLE Handle() const;
#endif
#ifdef __linux__
    int FileDescriptor() const;
#endif
public:
    //名字需要符合操作系统的名称规范
    //windows下的名称规范和windows的文件名规范基本一致，如果想跨session使用需要拥有管理员权限再名称前面加上"Global\"
    //linux需遵循文件名规范，不要使用特殊字符
    static zeus::expected<SharedMemory, std::error_code> OpenOrCreate(const std::string& name, uint64_t size, bool readOnly = false);
    static zeus::expected<SharedMemory, std::error_code> Open(const std::string& name, bool readOnly = false);
#ifdef __linux__
    static zeus::expected<void, std::error_code> Clear(const std::string& name);
#endif
protected:
    SharedMemory();
private:
    std::unique_ptr<SharedMemoryImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
