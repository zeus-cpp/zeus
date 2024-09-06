#include "zeus/foundation/file/file_utils.h"

#if _WIN32
#include <filesystem>
#include <cstring>
#include <cstdint>
#include <Windows.h>
#include "zeus/foundation/system/win/file_attributes.h"
#include "zeus/foundation/resource/win/handle.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/core/system_error.h"

namespace fs = std::filesystem;

namespace zeus
{

zeus::expected<void, std::error_code> CreateWriteableDirectory(const std::filesystem::path& path)
{
    SECURITY_ATTRIBUTES securittyAttr = {};
    securittyAttr.nLength             = sizeof(SECURITY_ATTRIBUTES);
    securittyAttr.bInheritHandle      = FALSE;
    SECURITY_DESCRIPTOR sd            = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    SetSecurityDescriptorGroup(&sd, nullptr, FALSE);
    SetSecurityDescriptorSacl(&sd, FALSE, nullptr, FALSE);
    securittyAttr.lpSecurityDescriptor = &sd;
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
    if (CreateDirectoryW(absoultePath.c_str(), &securittyAttr))
    {
        return {}; // 成功创建整个路径
    }
    const DWORD error = GetLastError();
    if (ERROR_ALREADY_EXISTS == error)
    {
        auto attribute = WinFileAttributes::Load(absoultePath);
        if (attribute.has_value())
        {
            if (!attribute->IsDirectory())
            {
                return zeus::unexpected(TranslateToSystemError(error));
            }
            attribute->SetReadOnly(false);
        }
        SetFileSecurityW(absoultePath.c_str(), DACL_SECURITY_INFORMATION, &sd);
        return {}; // 已经存在
    }
    else if (ERROR_PATH_NOT_FOUND == error)
    {
        const auto ret = CreateWriteableDirectory(absoultePath.parent_path());
        if (!ret.has_value())
        {
            return ret;
        }
        if (CreateDirectoryW(absoultePath.c_str(), &securittyAttr))
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
