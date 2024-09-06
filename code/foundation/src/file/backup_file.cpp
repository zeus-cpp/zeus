#include "zeus/foundation/file/backup_file.h"
#include <list>
#include <sstream>
#include <filesystem>
#include <fstream>
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/core/system_error.h"
namespace fs = std::filesystem;
namespace zeus
{
struct FileItem
{
    fs::path path;
    bool     text     = true;
    int32_t  priority = 0;
};
using FileList = std::list<FileItem>;
struct BackupFileImpl
{
    FileList files;
};

BackupFile::BackupFile(const std::filesystem::path &mainPath, bool text) : _impl(std::make_unique<BackupFileImpl>())
{
    FileItem item;
    item.path     = mainPath;
    item.priority = std::numeric_limits<int32_t>::min();
    item.text     = text;
    _impl->files.emplace_front(std::move(item));
}

BackupFile::~BackupFile()
{
}

void BackupFile::AddBackupFile(const std::filesystem::path &path, int32_t priority, bool text)
{
    FileItem item;
    item.path     = path;
    item.priority = priority;
    item.text     = text;
    auto iter     = _impl->files.begin();
    while (iter != _impl->files.end() && iter->priority <= priority)
    {
        ++iter;
    }
    _impl->files.emplace(iter, std::move(item));
}

void BackupFile::RemoveBackupFile(const std::filesystem::path &path)
{
    for (auto iter = _impl->files.begin(); iter != _impl->files.end(); ++iter)
    {
        if (iter->path == path)
        {
            _impl->files.erase(iter);
            break;
        }
    }
}

zeus::expected<void, std::error_code> BackupFile::SetContent(const void *data, size_t length)
{
    std::error_code error;
    for (const auto &iter : _impl->files)
    {
        CreateWriteableDirectory(iter.path.parent_path());
        std::ofstream file(iter.path, (iter.text ? (std::ios::out) : (std::ios::out | std::ios::binary)));
        if (file)
        {
            file.write(static_cast<const char *>(data), length);
        }
        else
        {
            error = GetLastSystemError();
        }
    }
    if (error)
    {
        return zeus::unexpected(TranslateToSystemError(error));
    }
    return {};
}

zeus::expected<void, std::error_code> BackupFile::SetContent(const std::string &data)
{
    return SetContent(data.data(), data.size());
}

zeus::expected<std::string, std::error_code> BackupFile::GetContent()
{
    std::error_code error;
    for (const auto &iter : _impl->files)
    {
        auto content = FileContent(iter.path, !iter.text);
        if (content.has_value())
        {
            return content;
        }
        else
        {
            error = content.error();
        }
    }
    return zeus::unexpected {TranslateToSystemError(error)};
}

} // namespace zeus
