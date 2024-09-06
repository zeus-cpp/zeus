#include <zeus/foundation/system/linux/xdg_base.h>

#include <filesystem>
#include <string>

#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/system/environment_variable.h>

namespace zeus
{

std::filesystem::path GetXdgConfigHome()
{
    auto const envConfigHome = EnvironmentVariable::Get("XDG_CONFIG_HOME");
    if (!envConfigHome.has_value())
    {
        std::filesystem::path home = Environment::GetHome();
        return home / ".config";
    }
    return std::filesystem::path(envConfigHome.value());
}

}
