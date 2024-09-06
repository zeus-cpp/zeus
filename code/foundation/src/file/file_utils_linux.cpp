#ifdef __linux__
#include "zeus/foundation/file/file_utils.h"
#include <filesystem>
#include <cstring>
#include <sys/stat.h>
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/core/system_error.h"

namespace fs = std::filesystem;

namespace zeus
{
zeus::expected<void, std::error_code> CreateWriteableDirectory(const std::filesystem::path& path)
{
    std::error_code ec;
    auto            absoultePath = fs::absolute(path, ec);
    if (ec)
    {
        return zeus::unexpected(ec);
    }
    absoultePath = fs::weakly_canonical(absoultePath, ec);
    if (ec)
    {
        return zeus::unexpected(ec);
    }
    if (0 == mkdir(absoultePath.c_str(), ACCESSPERMS))
    {
        return {}; // 成功创建整个路径
    }
    const auto error = errno;
    if (EEXIST == error)
    {
        if (!fs::is_directory(absoultePath, ec))
        {
            return zeus::unexpected(TranslateToSystemError(error));
        }
        chmod(absoultePath.c_str(), ACCESSPERMS);
        return {}; // 已经存在
    }
    else if (ENOENT == error)
    {
        const auto ret = CreateWriteableDirectory(absoultePath.parent_path());
        if (!ret.has_value())
        {
            return ret;
        }
        if (0 == mkdir(absoultePath.c_str(), ACCESSPERMS))
        {
            return {}; // 成功创建整个路径
        }
        else
        {
            return zeus::unexpected(GetLastSystemError());
        }
    }
    else
    {
        return zeus::unexpected(TranslateToSystemError(error));
    }
}
} // namespace zeus
#endif
