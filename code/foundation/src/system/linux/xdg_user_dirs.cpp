#include <zeus/foundation/system/linux/xdg_user_dirs.h>

#include <string>
#include <optional>
#include <filesystem>

#include <zeus/foundation/system/linux/xdg_base.h>
#include <zeus/foundation/file/kv_file_utils.h>
#include <zeus/foundation/system/environment.h>

namespace
{

constexpr std::string_view GetXdgUserDirName(zeus::XdgUserDirName name)
{
    switch (name)
    {
    case zeus::XdgUserDirName::DESKTOP:
        return "DESKTOP";
    case zeus::XdgUserDirName::DOWNLOAD:
        return "DOWNLOAD";
    case zeus::XdgUserDirName::TEMPLATES:
        return "TEMPLATES";
    case zeus::XdgUserDirName::PUBLICSHARE:
        return "PUBLICSHARE";
    case zeus::XdgUserDirName::DOCUMENTS:
        return "DOCUMENTS";
    case zeus::XdgUserDirName::MUSIC:
        return "MUSIC";
    case zeus::XdgUserDirName::PICTURES:
        return "PICTURES";
    case zeus::XdgUserDirName::VIDEOS:
        return "VIDEOS";
    default:
        return "";
    }
}

constexpr std::string_view GetXdgUserDirEnvName(zeus::XdgUserDirName name)
{
    switch (name)
    {
    case zeus::XdgUserDirName::DESKTOP:
        return "XDG_DESKTOP_DIR";
    case zeus::XdgUserDirName::DOWNLOAD:
        return "XDG_DOWNLOAD_DIR";
    case zeus::XdgUserDirName::TEMPLATES:
        return "XDG_TEMPLATES_DIR";
    case zeus::XdgUserDirName::PUBLICSHARE:
        return "XDG_PUBLICSHARE_DIR";
    case zeus::XdgUserDirName::DOCUMENTS:
        return "XDG_DOCUMENTS_DIR";
    case zeus::XdgUserDirName::MUSIC:
        return "XDG_MUSIC_DIR";
    case zeus::XdgUserDirName::PICTURES:
        return "XDG_PICTURES_DIR";
    case zeus::XdgUserDirName::VIDEOS:
        return "XDG_VIDEOS_DIR";
    default:
        return "";
    }
}

constexpr std::string_view GetFallbackDirRelatedToHome(zeus::XdgUserDirName name)
{
    switch (name)
    {
    case zeus::XdgUserDirName::DESKTOP:
        return "Desktop";
    case zeus::XdgUserDirName::DOWNLOAD:
        return "Downloads";
    case zeus::XdgUserDirName::TEMPLATES:
        return "Templates";
    case zeus::XdgUserDirName::PUBLICSHARE:
        return "Public";
    case zeus::XdgUserDirName::DOCUMENTS:
        return "Documents";
    case zeus::XdgUserDirName::MUSIC:
        return "Music";
    case zeus::XdgUserDirName::PICTURES:
        return "Pictures";
    case zeus::XdgUserDirName::VIDEOS:
        return "Videos";
    default:
        return "";
    }
}

std::filesystem::path GetFallbackDir(zeus::XdgUserDirName name)
{
    return zeus::Environment::GetHome() / GetFallbackDirRelatedToHome(name);
}

std::optional<std::string> GetDefaultDirRelatedToHome(zeus::XdgUserDirName name)
{
    static constexpr auto kConfigFile = "/etc/xdg/user-dirs.defaults";

    std::string value = zeus::GetKVFileValueUnquoted(kConfigFile, GetXdgUserDirName(name)).value_or("");
    if (value.empty())
    {
        return std::nullopt;
    }
    return value;
}

std::optional<std::filesystem::path> GetDefaultDir(zeus::XdgUserDirName name)
{
    std::optional<std::string> const value = GetDefaultDirRelatedToHome(name);
    if (!value.has_value())
    {
        return std::nullopt;
    }
    return zeus::Environment::GetHome() / std::filesystem::path(*value);
}

std::optional<std::filesystem::path> GetDirUserDirsDotDirs(zeus::XdgUserDirName name)
{
    std::filesystem::path const configHome = zeus::GetXdgConfigHome();
    std::filesystem::path const configFile = configHome / "user-dirs.dirs";
    std::string_view const      keyName    = GetXdgUserDirEnvName(name);
    std::string const           value      = zeus::GetKVFileValueUnquoted(configFile, keyName).value_or("");
    if (value.empty())
    {
        return std::nullopt;
    }
    if (value.find("$HOME/") == 0)
    {
        return zeus::Environment::GetHome() / std::filesystem::path(value.substr(6));
    }
    if (value.find("/") == 0)
    {
        return {value};
    }
    return std::nullopt;
}

} // namespace

namespace zeus
{

std::filesystem::path GetXdgUserDir(XdgUserDirName name)
{
    std::optional<std::filesystem::path> const dirUserDirsDotDirs = GetDirUserDirsDotDirs(name);
    if (dirUserDirsDotDirs.has_value())
    {
        return *dirUserDirsDotDirs;
    }

    std::optional<std::filesystem::path> const defaultDir = GetDefaultDir(name);
    if (defaultDir.has_value())
    {
        return *defaultDir;
    }

    return GetFallbackDir(name);
}

}
