#include "zeus/foundation/file/file_utils.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <ratio>
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/resource/file_mapping.h"
#include "zeus/foundation/core/system_error.h"

namespace fs = std::filesystem;

namespace zeus
{

namespace
{
zeus::expected<void, std::error_code> EnumDirectoryRegularFile(
    const std::filesystem::path& directory, const std::function<bool(const fs::directory_entry& entry)>& handler, bool recursive,
    bool followSymbolLinkDirectory, bool includeSymbolLinkFile
)
{
    std::set<std::filesystem::path> files;
    std::error_code                 ec;
    fs::directory_options           options = fs::directory_options::skip_permission_denied;
    if (followSymbolLinkDirectory)
    {
        options |= fs::directory_options::follow_directory_symlink;
    }
    if (recursive)
    {
        auto iterator = fs::recursive_directory_iterator(directory, options, ec);
        if (ec)
        {
            return zeus::unexpected {ec};
        }
        for (const auto& entry : iterator)
        {
            if (!includeSymbolLinkFile && entry.is_symlink(ec))
            {
                continue;
            }
            if (entry.is_regular_file(ec))
            {
                if (!handler(entry))
                {
                    return {};
                }
            }
        }
    }
    else
    {
        auto iterator = fs::directory_iterator(directory, options, ec);
        if (ec)
        {
            return zeus::unexpected {ec};
        }
        for (const auto& entry : iterator)
        {
            if (!includeSymbolLinkFile && entry.is_symlink(ec))
            {
                continue;
            }
            if (entry.is_regular_file(ec))
            {
                if (!handler(entry))
                {
                    return {};
                }
            }
        }
    }
    return {};
}
} // namespace

zeus::expected<void, std::error_code> FileEachLine(
    const std::filesystem::path& path, const std::function<bool(std::string_view line)>& callback, char delim
)
{
    std::ifstream file(path);
    if (file)
    {
        std::string line;
        while (std::getline(file, line, delim))
        {
            if (!callback(line))
            {
                break;
            }
        }
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<void, std::error_code> FileEachLine(const std::filesystem::path& path, const std::function<bool(std::string_view line)>& callback)
{
    std::ifstream file(path);
    if (file)
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (!callback(line))
            {
                break;
            }
        }
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<void, std::error_code> FileEachLineKVData(
    const std::filesystem::path& path, std::string_view delim, const std::function<bool(std::string_view key, std::string_view value)>& callback,
    const std::set<std::string>& ignorePrefixs
)
{
    return FileEachLine(
        path,
        [&callback, &delim, &ignorePrefixs](std::string_view line)
        {
            auto lineData = TrimBegin(line);
            if (lineData.empty())
            {
                return true;
            }
            for (const auto& ignorePrefix : ignorePrefixs)
            {
                if (StartWith(lineData, ignorePrefix))
                {
                    return true;
                }
            }
            const auto pos = lineData.find(delim);
            if (pos == std::string_view::npos)
            {
                return true;
            }
            const auto key   = lineData.substr(0, pos);
            const auto value = lineData.substr(pos + delim.size());
            return callback(key, value);
        }
    );
}

zeus::expected<void, std::error_code> FileEachLineKVData(
    const std::filesystem::path& path, std::string_view delim, const std::function<bool(std::string_view key, std::string_view value)>& callback,
    const std::function<bool(std::string_view line)>& invalidCallback, const std::set<std::string>& ignorePrefixs
)
{
    return FileEachLine(
        path,
        [&callback, &invalidCallback, &delim, &ignorePrefixs](std::string_view line)
        {
            auto lineData = TrimBegin(line);
            if (lineData.empty())
            {
                return true;
            }
            for (const auto& ignorePrefix : ignorePrefixs)
            {
                if (StartWith(lineData, ignorePrefix))
                {
                    return true;
                }
            }
            const auto pos = lineData.find(delim);
            if (pos == std::string_view::npos)
            {
                invalidCallback(line);
            }
            else
            {
                const auto key   = lineData.substr(0, pos);
                const auto value = lineData.substr(pos + delim.size());
                return callback(key, value);
            }
            return true;
        }
    );
}

zeus::expected<std::string, std::error_code> FileContent(const std::filesystem::path& path, bool binary)
{
    std::stringstream stream;
    std::ifstream     file(path, binary ? (std::ios::in | std::ios::binary) : std::ios::in);
    if (!file)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    stream << file.rdbuf();
    return stream.str();
}

zeus::expected<uint64_t, std::error_code> GetDirectoryRegularFileSize(
    const std::filesystem::path& directory, bool recursive, bool followSymbolLinkDirectory, bool includeSymbolLinkFile
)
{
    uint64_t totalSize = 0;
    auto     ret       = EnumDirectoryRegularFile(
        directory,
        [&totalSize](const fs::directory_entry& entry)
        {
            std::error_code ec;
            auto            size = entry.file_size(ec);
            if (!ec)
            {
                totalSize += size;
            }
            return true;
        },
        recursive, followSymbolLinkDirectory, includeSymbolLinkFile
    );
    if (ret.has_value())
    {
        return totalSize;
    }
    else
    {
        return zeus::unexpected {ret.error()};
    }
}

zeus::expected<std::set<std::filesystem::path>, std::error_code> GetDirectoryRegularFiles(
    const std::filesystem::path& directory, const std::function<bool(const std::filesystem::path& path)>& filter, bool recursive,
    bool followSymbolLinkDirectory, bool includeSymbolLinkFile
)
{
    std::set<std::filesystem::path> files;
    auto                            ret = EnumDirectoryRegularFile(
        directory,
        [&files, &filter](const fs::directory_entry& entry)
        {
            if (filter(entry.path()))
            {
                files.emplace(entry.path());
            }
            return true;
        },
        recursive, followSymbolLinkDirectory, includeSymbolLinkFile
    );
    if (ret.has_value())
    {
        return files;
    }
    else
    {
        return zeus::unexpected {ret.error()};
    }
}

zeus::expected<std::set<std::filesystem::path>, std::error_code> GetDirectoryRegularFiles(
    const std::filesystem::path& directory, bool recursive, bool followSymbolLinkDirectory, bool includeSymbolLinkFile
)
{
    std::set<std::filesystem::path> files;
    auto                            ret = EnumDirectoryRegularFile(
        directory,
        [&files](const fs::directory_entry& entry)
        {
            files.emplace(entry.path());
            return true;
        },
        recursive, followSymbolLinkDirectory, includeSymbolLinkFile
    );
    if (ret.has_value())
    {
        return files;
    }
    else
    {
        return zeus::unexpected {ret.error()};
    }
}

zeus::expected<std::set<std::filesystem::path>, std::error_code> GetDirectoryRegularFiles(
    const std::filesystem::path& directory, const std::string& suffix, bool recursive, bool followSymbolLinkDirectory, bool includeSymbolLinkFile
)
{
    std::function<bool(const std::filesystem::path& path)> filter;

    std::string extension;
#ifdef _WIN32

    if (!StartWith(suffix, "."))
    {
        extension = "." + ToUpperCopy(suffix);
    }
    else
    {
        extension = ToUpperCopy(suffix);
    }
    filter = [&extension](const std::filesystem::path& path)
    {
        return extension == ToUpperCopy(path.extension().u8string());
    };
#else
    if (!StartWith(suffix, "."))
    {
        extension = "." + suffix;
    }
    else
    {
        extension = suffix;
    }
    filter = [&extension](const std::filesystem::path& path)
    {
        return extension == path.extension().u8string();
    };
#endif
    return GetDirectoryRegularFiles(directory, filter, recursive, followSymbolLinkDirectory, includeSymbolLinkFile);
}

zeus::expected<bool, std::error_code> FileEqual(std::ifstream& file1, std::ifstream& file2)
{
    static const size_t BUFF_SIZE = 1024;
    char                buffer1[BUFF_SIZE];
    char                buffer2[BUFF_SIZE];
    while (!file1.eof())
    {
        file1.read(buffer1, BUFF_SIZE);
        file2.read(buffer2, BUFF_SIZE);
        auto count1 = file1.gcount();
        auto count2 = file2.gcount();
        if (count1 != count2)
        {
            return false;
        }
        if (std::memcmp(buffer1, buffer2, count1))
        {
            return false;
        }
    }
    if (!file2.eof())
    {
        return false;
    }
    return true;
}

zeus::expected<bool, std::error_code> FileEqual(const std::filesystem::path& path1, const std::filesystem::path& path2)
{
    static constexpr uint64_t kMappingSize = 2 * 1024 * 1024;
    std::ifstream             file1;
    std::ifstream             file2;
    std::error_code           ec;
    uint64_t                  size1 = fs::file_size(path1, ec);
    if (ec)
    {
        return zeus::unexpected(ec);
    }
    uint64_t size2 = fs::file_size(path2, ec);
    if (ec)
    {
        return zeus::unexpected(ec);
    }
    if (size1 != size2)
    {
        return false;
    }
    uint64_t size = size1;
    if (0 == size)
    {
        return true;
    }
    if (size <= kMappingSize)
    {
        auto mapping1 = FileMapping::Create(path1, false);
        if (!mapping1)
        {
            return zeus::unexpected(mapping1.error());
        }
        auto mapping2 = FileMapping::Create(path2, false);
        if (!mapping2)
        {
            return zeus::unexpected(mapping2.error());
        }
        if (const auto ret = mapping1->MapAll(); !ret.has_value())
        {
            return zeus::unexpected(ret.error());
        }
        if (const auto ret = mapping2->MapAll(); !ret.has_value())
        {
            return zeus::unexpected(ret.error());
        }
        return 0 == std::memcmp(mapping1->Data(), mapping2->Data(), size);
    }
    else
    {
        auto mapping1 = FileMapping::Create(path1, false);
        if (!mapping1)
        {
            return zeus::unexpected(mapping1.error());
        }
        auto mapping2 = FileMapping::Create(path2, false);
        if (!mapping2)
        {
            return zeus::unexpected(mapping2.error());
        }
        const uint64_t count = size / kMappingSize;

        for (uint64_t index = 0; index <= count; ++index)
        {
            if (const auto ret = mapping1->Map(index * kMappingSize, count == index ? (size % kMappingSize) : kMappingSize); !ret.has_value())
            {
                return zeus::unexpected(ret.error());
            }
            if (const auto ret = mapping2->Map(index * kMappingSize, count == index ? (size % kMappingSize) : kMappingSize); !ret.has_value())
            {
                return zeus::unexpected(ret.error());
            }
            if (0 != std::memcmp(mapping1->Data(), mapping2->Data(), mapping1->Size()))
            {
                return false;
            }
        }
        return true;
    }
}

} // namespace zeus
