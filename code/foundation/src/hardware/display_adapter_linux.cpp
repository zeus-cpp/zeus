#include "zeus/foundation/hardware/display_adapter.h"

#ifdef __linux__
#include <filesystem>
#include <string>
#include <optional>
#include <fstream>
#include <map>
#include <cinttypes>
#include <cstdint>
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/file/file_utils.h"
#include "impl/display_adapter_impl.h"

namespace fs = std::filesystem;

namespace zeus::Hardware
{
std::string GetDisplayPCIDeviceName(
    const std::string& vendorId, const std::string& deviceId, const std::string& subdeviceVendorId, std::string& subdeviceId
);
namespace
{

std::map<std::string, std::string> INTELDisplayDeviceTable = {

    {"", ""},
};
std::map<std::string, std::string> ATIDisplayDeviceTable;
std::map<std::string, std::string> NVIDIADisplayDeviceTable;

struct PCINode
{
    unsigned int domian   = 0;
    unsigned int bus      = 0;
    unsigned int device   = 0;
    int          function = 0;
};

std::optional<PCINode> ParseNodePath(const fs::path& path)
{
    PCINode node;
    if (sscanf(path.filename().c_str(), "%x:%x:%x.%d", &node.domian, &node.bus, &node.device, &node.function) == 4)
    {
        return node;
    }
    return std::nullopt;
}

bool IsVGAController(const fs::path& path)
{
    auto classValue = Trim(FileContent(path / "class", false).value_or(""));
    return classValue == "0x030000";
}

std::string MatchDeviceName(const fs::path& path)
{
    auto vendorId = Trim(FileContent(path / "vendor", false).value_or(""));
    auto deviceId = Trim(FileContent(path / "device", false).value_or(""));

    auto subdeviceVendorId = Trim(FileContent(path / "subsystem_vendor", false).value_or(""));
    auto subdeviceDeviceId = Trim(FileContent(path / "subsystem_device", false).value_or(""));
    if (vendorId.empty() || deviceId.empty())
    {
        return "Unknown Display Adapter";
    }
    return GetDisplayPCIDeviceName(vendorId, deviceId, subdeviceVendorId, subdeviceDeviceId);
}

uint64_t ParseMemoryByResource(const fs::path& path)
{
    //目前没有办法准确判断哪个是显存，从观测结果来看最大的那个是显存
    constexpr uint64_t PCI_BASE_ADDRESS_SPACE_IO = 0x01;
    std::ifstream      resource(path / "resource");
    uint64_t           maxMemorySize = 0;
    if (resource)
    {
        std::string line;
        while (std::getline(resource, line))
        {
            uint64_t start = 0;
            uint64_t end   = 0;
            uint64_t flag  = 0;
            if ((sscanf(line.c_str(), "%" SCNx64 "%" SCNx64 "%" SCNx64, &start, &end, &flag) == 3) && !(flag & PCI_BASE_ADDRESS_SPACE_IO))
            {
                auto size = end - start;
                if (size > maxMemorySize)
                {
                    maxMemorySize = size;
                }
            }
        }
    }
    return maxMemorySize;
}

} // namespace

std::vector<DisplayAdapter> DisplayAdapter::ListAll()
{
    std::vector<DisplayAdapter> result;
    std::error_code             ec;
    for (auto& item : fs::directory_iterator("/sys/bus/pci/devices", ec))
    {
        if (!item.is_directory(ec))
        {
            continue;
        }
        auto nodePath = item.path();
        auto node     = ParseNodePath(nodePath);
        if (!node)
        {
            continue;
        }
        if (!IsVGAController(nodePath))
        {
            continue;
        }

        const auto&    adapterDir = item.path();
        DisplayAdapter adapter;
        adapter._impl->describeName = MatchDeviceName(adapterDir);
        adapter._impl->videoMemory  = ParseMemoryByResource(adapterDir) / 1024;
        result.emplace_back(std::move(adapter));
    }
    return result;
}

} // namespace zeus::Hardware
#endif
