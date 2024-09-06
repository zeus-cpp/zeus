#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include "zeus/expected.hpp"
namespace zeus
{

struct BackupFileImpl;
class BackupFile
{
public:
    BackupFile(const std::filesystem::path& mainPath, bool text = true);
    ~BackupFile();
    BackupFile(const BackupFile&)                                             = delete;
    BackupFile(BackupFile&&)                                                  = delete;
    BackupFile&                                  operator=(const BackupFile&) = delete;
    BackupFile&                                  operator=(BackupFile&&)      = delete;
    void                                         AddBackupFile(const std::filesystem::path& path, int32_t priority = 0, bool text = true);
    void                                         RemoveBackupFile(const std::filesystem::path& path);
    zeus::expected<void, std::error_code>        SetContent(const void* data, size_t length);
    zeus::expected<void, std::error_code>        SetContent(const std::string& data);
    zeus::expected<std::string, std::error_code> GetContent();
private:
    std::unique_ptr<BackupFileImpl> _impl;
};
}

#include "zeus/foundation/core/zeus_compatible.h"
