#pragma once
#ifdef _WIN32
#include <vector>
#include <memory>
#include <system_error>
#include <zeus/expected.hpp>
struct _GUID;
namespace zeus
{
struct WinDeviceSetupImpl;
class WinDeviceSetup
{
public:
    //这里只列出了常见的设备类型，其他类型请自行查找GUID
    enum class DeviceType
    {
        kBATTERY,
        kBLUETOOTH,
        kCAMERA, //注意 在win10rs3之后CAMERA才被单独归类，此前摄像头归属IMAGE
        kCDROM,
        kCOMPUTER,
        kDECODER,
        kDISKDRIVE,
        kDISPLAY,
        kDOT4,
        kFIRMWARE,
        kFLOPPYDISK,
        kGPS,
        kHDC,
        kHIDCLASS,
        kIMAGE,
        kKEYBOARD,
        kMEDIA,
        kMEDIUM_CHANGER,
        kMEMORY,
        kMODEM,
        kMONITOR,
        kMOUSE,
        kNET,
        kNETCLIENT,
        kNETDRIVER,
        kNETSERVICE,
        kNETTRANS,
        kNODRIVER,
        kPRINTER,
        kPROCESSOR,
        kSOFTWARECOMPONENT,
        kSOUND,
        kSYSTEM,
        kUNKNOWN,
        kUSB,
        kVOLUME,
        kVOLUMESNAPSHOT,
        kWCEUSBS,
        kWPD,
    };
    ~WinDeviceSetup();
    WinDeviceSetup(const WinDeviceSetup& other);
    WinDeviceSetup(WinDeviceSetup&& other) noexcept;
    WinDeviceSetup&                                           operator=(const WinDeviceSetup& other);
    WinDeviceSetup&                                           operator=(WinDeviceSetup&& other) noexcept;
    //内部实现使用SetupDiGetDeviceRegistryPropertyW,所以字符串数据为UNICODE
    zeus::expected<std::vector<uint8_t>, std::error_code>     GetRegistryProperty(unsigned long property);
    zeus::expected<std::vector<std::string>, std::error_code> GetRegistryHardwareId();
    zeus::expected<std::string, std::error_code>              GetRegistryDeviceDesc();
    zeus::expected<std::string, std::error_code>              GetRegistryManufacturer();
    zeus::expected<std::string, std::error_code>              GetRegistryFriendlyName();
    zeus::expected<size_t, std::error_code>                   GetRegistryBusNumber();
    zeus::expected<std::string, std::error_code>              GetRegistryDeviceObjectName();
    zeus::expected<std::string, std::error_code>              GetRegistryService();
    zeus::expected<std::string, std::error_code>              GetDeviceInstancePath();
    unsigned long                                             GetDevInst();
    // CM_Get_DevNode_Status
    zeus::expected<void, std::error_code>                     GetDeviceNodeStatus(unsigned long& status, unsigned long& problem);
    zeus::expected<bool, std::error_code>                     IsDisable();
    zeus::expected<bool, std::error_code>                     IsRemovable();

public:
    static std::vector<WinDeviceSetup> List(DeviceType type, bool present = true, bool deviceInterface = false);
    static std::vector<WinDeviceSetup> List(const _GUID* guid, bool present = true, bool deviceInterface = false);

protected:
    WinDeviceSetup();

private:
    std::unique_ptr<WinDeviceSetupImpl> _impl;
};
} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
