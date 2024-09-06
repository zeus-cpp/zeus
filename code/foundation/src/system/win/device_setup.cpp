#include "zeus/foundation/system/win/device_setup.h"
#ifdef _WIN32
#include <windows.h>
#include <SetupAPI.h>
#include <Cfgmgr32.h>
#include <devguid.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/system/environment.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/byte/byte_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{
namespace
{
GUID GUID_DEVCLASS_CAMERA = {0xca3e7ab9L, 0xb4c3, 0x4ae6, 0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f};
}

class DevClassInfo
{
public:
    DevClassInfo(HDEVINFO handle) : _handle(handle) {}

    ~DevClassInfo()
    {
        if (_handle)
        {
            SetupDiDestroyDeviceInfoList(_handle);
        }
    }

    HANDLE Handle() const { return _handle; }
    bool   Empty() const { return _handle == nullptr; }

    explicit operator bool() const { return !Empty(); }

    operator HDEVINFO() const { return _handle; }

private:
    HDEVINFO _handle;
};

struct WinDeviceSetupImpl
{
    std::shared_ptr<DevClassInfo> devClass;
    SP_DEVINFO_DATA               infoData = {};
};

WinDeviceSetup::WinDeviceSetup() : _impl(std::make_unique<WinDeviceSetupImpl>())
{
}

WinDeviceSetup::WinDeviceSetup(const WinDeviceSetup& other) : _impl(std::make_unique<WinDeviceSetupImpl>(*other._impl))
{
}

WinDeviceSetup::WinDeviceSetup(WinDeviceSetup&& other) noexcept : _impl(std::move(other._impl))
{
}

WinDeviceSetup& WinDeviceSetup::operator=(const WinDeviceSetup& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

WinDeviceSetup& WinDeviceSetup::operator=(WinDeviceSetup&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

WinDeviceSetup::~WinDeviceSetup()
{
}

zeus::expected<std::vector<uint8_t>, std::error_code> WinDeviceSetup::GetRegistryProperty(unsigned long property)
{
    DWORD requiredSize = 0;
    SetupDiGetDeviceRegistryPropertyW(*_impl->devClass, &_impl->infoData, property, nullptr, nullptr, 0, &requiredSize);
    const auto error = GetLastError();
    if (error == ERROR_INSUFFICIENT_BUFFER)
    {
        std::vector<uint8_t> data(requiredSize);
        if (SetupDiGetDeviceRegistryPropertyW(*_impl->devClass, &_impl->infoData, property, nullptr, data.data(), requiredSize, &requiredSize))
        {
            return std::move(data);
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
zeus::expected<std::vector<std::string>, std::error_code> WinDeviceSetup::GetRegistryHardwareId()
{
    auto data = GetRegistryProperty(SPDRP_HARDWAREID);
    if (data.has_value())
    {
        const auto* end   = L"";
        auto        split = zeus::ByteSplit({data->data(), data->size()}, {reinterpret_cast<const uint8_t*>(end), sizeof(wchar_t)}, sizeof(wchar_t));
        std::vector<std::string> ids;
        ids.reserve(split.size());
        for (auto& slice : split)
        {
            ids.emplace_back(zeus::CharsetUtils::UnicodeToUTF8(std::wstring(
                reinterpret_cast<const wchar_t*>(slice.Data()), reinterpret_cast<const wchar_t*>(slice.Data()) + slice.Size() / sizeof(wchar_t)
            )));
        }
        return ids;
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<std::string, std::error_code> WinDeviceSetup::GetRegistryDeviceDesc()
{
    auto data = GetRegistryProperty(SPDRP_DEVICEDESC);
    if (data.has_value())
    {
        return zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<wchar_t*>(data->data()));
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<std::string, std::error_code> WinDeviceSetup::GetRegistryManufacturer()
{
    auto data = GetRegistryProperty(SPDRP_MFG);
    if (data.has_value())
    {
        return zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<wchar_t*>(data->data()));
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<std::string, std::error_code> WinDeviceSetup::GetRegistryFriendlyName()
{
    auto data = GetRegistryProperty(SPDRP_FRIENDLYNAME);
    if (data.has_value())
    {
        return zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<wchar_t*>(data->data()));
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<size_t, std::error_code> WinDeviceSetup::GetRegistryBusNumber()
{
    auto data = GetRegistryProperty(SPDRP_BUSNUMBER);
    if (data.has_value())
    {
        assert(data->size() == sizeof(DWORD));
        return *(reinterpret_cast<DWORD*>(data->data()));
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<std::string, std::error_code> WinDeviceSetup::GetRegistryDeviceObjectName()
{
    auto data = GetRegistryProperty(SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
    if (data.has_value())
    {
        return zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<wchar_t*>(data->data()));
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<std::string, std::error_code> WinDeviceSetup::GetRegistryService()
{
    auto data = GetRegistryProperty(SPDRP_SERVICE);
    if (data.has_value())
    {
        return zeus::CharsetUtils::UnicodeToUTF8(reinterpret_cast<wchar_t*>(data->data()));
    }
    else
    {
        return zeus::unexpected(data.error());
    }
}
zeus::expected<std::string, std::error_code> WinDeviceSetup::GetDeviceInstancePath()
{
    ULONG     size = 0;
    CONFIGRET ret  = CM_Get_Device_ID_Size(&size, _impl->infoData.DevInst, 0);
    if (CR_SUCCESS == ret)
    {
        if (size)
        {
            ++size; // add terminating nullptr buffer
            auto buffer = std::make_unique<wchar_t[]>(size);
            std::memset(buffer.get(), 0, sizeof(wchar_t) * (size));
            ret = CM_Get_Device_IDW(_impl->infoData.DevInst, buffer.get(), size, 0);
            if (CR_SUCCESS == ret)
            {
                return zeus::CharsetUtils::UnicodeToUTF8(buffer.get());
            }
            else
            {
                return zeus::unexpected(SystemError {ret});
            }
        }
        else
        {
            return {};
        }
    }
    else
    {
        return zeus::unexpected(SystemError {ret});
    }
}
unsigned long WinDeviceSetup::GetDevInst()
{
    return _impl->infoData.DevInst;
}
zeus::expected<void, std::error_code> WinDeviceSetup::GetDeviceNodeStatus(unsigned long& status, unsigned long& problem)
{
    const auto ret = CM_Get_DevNode_Status(&status, &problem, _impl->infoData.DevInst, 0);
    if (ret)
    {
        return {};
    }
    else
    {
        return zeus::unexpected(GetLastSystemError());
    }
}
zeus::expected<bool, std::error_code> WinDeviceSetup::IsDisable()
{
    DWORD      problem   = 0;
    DWORD      devStatus = 0;
    const auto ret       = GetDeviceNodeStatus(devStatus, problem);
    if (ret)
    {
        bool disable = (devStatus & DN_HAS_PROBLEM) && (CM_PROB_HARDWARE_DISABLED == problem || CM_PROB_DISABLED == problem);
        return disable;
    }
    else
    {
        return zeus::unexpected(ret.error());
    }
}
zeus::expected<bool, std::error_code> WinDeviceSetup::IsRemovable()
{
    DWORD      problem   = 0;
    DWORD      devStatus = 0;
    const auto ret       = GetDeviceNodeStatus(devStatus, problem);
    if (ret)
    {
        bool removable = (devStatus & DN_REMOVABLE);
        return removable;
    }
    else
    {
        return zeus::unexpected(ret.error());
    }
}

std::vector<WinDeviceSetup> WinDeviceSetup::List(DeviceType type, bool present, bool deviceInterface)
{
    const GUID* guid = nullptr;
    switch (type)
    {
    case DeviceType::kBATTERY:
        guid = &GUID_DEVCLASS_BATTERY;
        break;
    case DeviceType::kBLUETOOTH:
        guid = &GUID_DEVCLASS_BLUETOOTH;
        break;
    case DeviceType::kCAMERA:
        guid = &GUID_DEVCLASS_CAMERA;
        break;
    case DeviceType::kCDROM:
        guid = &GUID_DEVCLASS_CDROM;
        break;
    case DeviceType::kCOMPUTER:
        guid = &GUID_DEVCLASS_COMPUTER;
        break;
    case DeviceType::kDECODER:
        guid = &GUID_DEVCLASS_DECODER;
        break;
    case DeviceType::kDISKDRIVE:
        guid = &GUID_DEVCLASS_DISKDRIVE;
        break;
    case DeviceType::kDISPLAY:
        guid = &GUID_DEVCLASS_DISPLAY;
        break;
    case DeviceType::kDOT4:
        guid = &GUID_DEVCLASS_DOT4;
        break;
    case DeviceType::kFIRMWARE:
        guid = &GUID_DEVCLASS_FIRMWARE;
        break;
    case DeviceType::kFLOPPYDISK:
        guid = &GUID_DEVCLASS_FLOPPYDISK;
        break;
    case DeviceType::kGPS:
        guid = &GUID_DEVCLASS_GPS;
        break;
    case DeviceType::kHDC:
        guid = &GUID_DEVCLASS_HDC;
        break;
    case DeviceType::kHIDCLASS:
        guid = &GUID_DEVCLASS_HIDCLASS;
        break;

    case DeviceType::kIMAGE:
        guid = &GUID_DEVCLASS_IMAGE;
        break;
    case DeviceType::kKEYBOARD:
        guid = &GUID_DEVCLASS_KEYBOARD;
        break;
    case DeviceType::kMEDIA:
        guid = &GUID_DEVCLASS_MEDIA;
        break;
    case DeviceType::kMEDIUM_CHANGER:
        guid = &GUID_DEVCLASS_MEDIUM_CHANGER;
        break;
    case DeviceType::kMEMORY:
        guid = &GUID_DEVCLASS_MEMORY;
        break;
    case DeviceType::kMODEM:
        guid = &GUID_DEVCLASS_MODEM;
        break;
    case DeviceType::kMONITOR:
        guid = &GUID_DEVCLASS_MONITOR;
        break;
    case DeviceType::kMOUSE:
        guid = &GUID_DEVCLASS_MOUSE;
        break;
    case DeviceType::kNET:
        guid = &GUID_DEVCLASS_NET;
        break;
    case DeviceType::kNETCLIENT:
        guid = &GUID_DEVCLASS_NETCLIENT;
        break;
    case DeviceType::kNETDRIVER:
        // guid = &GUID_DEVCLASS_NETDRIVER;
        break;
    case DeviceType::kNETSERVICE:
        guid = &GUID_DEVCLASS_NETSERVICE;
        break;
    case DeviceType::kNETTRANS:
        guid = &GUID_DEVCLASS_NETTRANS;
        break;
    case DeviceType::kNODRIVER:
        guid = &GUID_DEVCLASS_NODRIVER;
        break;
    case DeviceType::kPRINTER:
        guid = &GUID_DEVCLASS_PRINTER;
        break;
    case DeviceType::kPROCESSOR:
        guid = &GUID_DEVCLASS_PROCESSOR;
        break;
    case DeviceType::kSOFTWARECOMPONENT:
        // guid = &GUID_DEVCLASS_SOFTWARECOMPONENT;
        break;
    case DeviceType::kSOUND:
        guid = &GUID_DEVCLASS_SOUND;
        break;
    case DeviceType::kSYSTEM:
        guid = &GUID_DEVCLASS_SYSTEM;
        break;
    case DeviceType::kUNKNOWN:
        guid = &GUID_DEVCLASS_UNKNOWN;
        break;
    case DeviceType::kUSB:
        guid = &GUID_DEVCLASS_USB;
        break;
    case DeviceType::kVOLUME:
        guid = &GUID_DEVCLASS_VOLUME;
        break;
    case DeviceType::kVOLUMESNAPSHOT:
        guid = &GUID_DEVCLASS_VOLUMESNAPSHOT;
        break;
    case DeviceType::kWCEUSBS:
        guid = &GUID_DEVCLASS_WCEUSBS;
        break;
    case DeviceType::kWPD:
        guid = &GUID_DEVCLASS_WPD;
        break;
    default:
        break;
    }
    return List(guid, present, deviceInterface);
}

std::vector<WinDeviceSetup> WinDeviceSetup::List(const _GUID* guid, bool present, bool deviceInterface)
{
    std::vector<WinDeviceSetup> result;
    if (guid)
    {
        auto devClass = std::make_shared<DevClassInfo>(
            SetupDiGetClassDevsW(guid, nullptr, nullptr, (present ? DIGCF_PRESENT : 0) | (deviceInterface ? DIGCF_DEVICEINTERFACE : 0))
        );
        if (devClass)
        {
            SP_DEVINFO_DATA devInfo;
            devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
            DWORD index    = 0;
            while (SetupDiEnumDeviceInfo(*devClass, index, &devInfo))
            {
                auto impl      = std::make_unique<WinDeviceSetupImpl>();
                impl->devClass = devClass;
                impl->infoData = devInfo;
                WinDeviceSetup setup;
                setup._impl.swap(impl);
                result.emplace_back(std::move(setup));
                index++;
            }
        }
    }
    return result;
}
} // namespace zeus
#endif
