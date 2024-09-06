#include <filesystem>

namespace zeus
{

enum class XdgUserDirName
{
    DESKTOP,
    DOWNLOAD,
    TEMPLATES,
    PUBLICSHARE,
    DOCUMENTS,
    MUSIC,
    PICTURES,
    VIDEOS,
};

std::filesystem::path GetXdgUserDir(XdgUserDirName name);

}

#include "zeus/foundation/core/zeus_compatible.h"
