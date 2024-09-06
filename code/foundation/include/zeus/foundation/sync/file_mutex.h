#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <system_error>
#include "zeus/expected.hpp"

namespace zeus
{
struct FileMutexImpl;
class FileMutex
{
public:

    FileMutex(const FileMutex &)            = delete;
    FileMutex &operator=(const FileMutex &) = delete;
    FileMutex(FileMutex &&other) noexcept;
    FileMutex &operator=(FileMutex &&other) noexcept;
    ~FileMutex();
    std::filesystem::path                 Path() const;
    zeus::expected<void, std::error_code> LockExclusive();
    zeus::expected<void, std::error_code> LockShared();
    zeus::expected<void, std::error_code> Unlock();
    zeus::expected<bool, std::error_code> TryLockExclusive();
    zeus::expected<bool, std::error_code> TryLockShared();
    //以下为兼容std::mutex的接口
    void                                  lock();
    void                                  lock_shared();
    void                                  unlock();
    bool                                  try_lock();
    bool                                  try_lock_shared();
public:
    static zeus::expected<FileMutex, std::error_code> Create(const std::filesystem::path &filePath);
protected:
    FileMutex();
private:
    std::unique_ptr<FileMutexImpl> _impl;
};
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
