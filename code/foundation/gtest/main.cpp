#include <gtest/gtest.h>
#include <zeus/foundation/debugger/debugger.h>
#include <zeus/foundation/string/unit_conversion.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/string/charset_utils.h>
#include <zeus/foundation/system/os.h>
#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/system/net_adapter.h>
#include <zeus/foundation/hardware/memory.h>
#include <zeus/foundation/hardware/cpu.h>
#include <zeus/foundation/hardware/display_adapter.h>
#include <zeus/foundation/hardware/storage.h>
#include <zeus/foundation/hardware/hard_disk.h>
#include <zeus/foundation/hardware/monitor.h>
#include <zeus/foundation/hardware/system.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/system/system_notify.h>
#include <zeus/foundation/time/time_utils.h>
#include <zeus/foundation/thread/thread_pool.h>

using namespace zeus;

TEST(macId, performance)
{
    auto       begin = std::chrono::steady_clock::now();
    const auto count = 100;
    for (int i = 0; i < count; i++)
    {
        auto ret = Environment::GetMacId();
    }
    auto end  = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    EXPECT_TRUE(diff.count());
    std::cout << "run " << count << " times GetMacId cost " << diff.count() << "ms" << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "[EXE]:" << CurrentExe::GetAppPathString() << std::endl;
    std::cout << "[EXE CMDLINE]:" << Join(*Process::GetProcessCmdlineArguments(CurrentExe::GetProcessId()), " ") << std::endl;
    std::cout << "[DEBUGGER]:" << std::boolalpha << zeus::IsDebuggerAttached() << std::endl;
    std::cout << "[PID]:" << CurrentExe::GetProcessId() << std::endl;
#ifdef __linux__
    std::cout << "[USER ID]:" << CurrentExe::GetUserId() << std::endl;
    std::cout << "[GROUP ID]:" << CurrentExe::GetGroupId() << std::endl;
#endif
    std::cout << "[USER NAME]:" << CurrentExe::GetUsername() << std::endl;
    std::cout << "[HOME]:" << Environment::GetHomeString() << std::endl;
    std::cout << "[MACHINE NAME]:" << Environment::GetMachineName() << std::endl;
#ifdef __linux__
    std::cout << "[ROOT]:" << std::boolalpha << CurrentExe::HasRootPermission() << std::endl;
#endif
    std::cout << std::endl;

    std::cout << "[OS KERNEL]:" << OS::OsKernelName() << std::endl;
    std::cout << "[OS KERNEL VERSION]:" << OS::OsKernelVersionString() << std::endl;
    std::cout << "[OS ARCHITECTURE]:" << OS::OsArchitecture() << std::endl;
    std::cout << "[OS NAME]:" << OS::OsDisplayName() << std::endl;
    std::cout << "[OS FULL]:" << OS::OsFullName() << std::endl;
    std::cout << "[OS VERSION STRING]:" << OS::OsVersionString() << std::endl;
    std::cout << "[OS VERSION]:" << OS::OsVersion().ToString() << std::endl;
    std::cout << "[OS TYPE]:" << OS::OsProductType() << std::endl;
    std::cout << "[OS BOOT TIME]:" << FormatSystemClockTime(OS::OsBootTime(), "%Y-%m-%d:%H:%M:%S") << std::endl;
    std::cout << std::endl;

    std::cout << "[ENV CPU CORE]:" << Environment::ProcessorCoreCount() << std::endl;
    std::cout << "[ENV CPU THREAD]:" << Environment::ProcessorCount() << std::endl;
    std::cout << std::endl;

    auto memory = Hardware::Memory::GetMemory(true);
    std::cout << "[MEMORY VISIBLE]:" << UnitConversion::CapacityKUnitCast(memory.VisiblePhysicalCapacity()).ToCapacityString() << std::endl;
    std::cout << "[MEMORY INSTALLED]:" << UnitConversion::CapacityKUnitCast(memory.InstalledPhysicalCapacity()).ToCapacityString() << std::endl;
    std::cout << "[MEMORY FREE]:" << UnitConversion::CapacityKUnitCast(memory.FreePhysicalCapacity()).ToCapacityString() << std::endl;
    std::cout << "[MEMORY PAGE]:" << UnitConversion::CapacityKUnitCast(memory.TotalPageCapacity()).ToCapacityString() << std::endl;
    std::cout << std::endl;

    auto cpu = Hardware::Cpu::ListAll();
    std::cout << "[CPU NAME]:" << cpu[0].Name() << std::endl;
    std::cout << "[CPU CORE]:" << cpu[0].PhysicalCoreCount() << std::endl;
    std::cout << "[CPU THREAD]:" << cpu[0].LogicalCoreCount() << std::endl;
    std::cout << "[CPU USAGE]:" << Hardware::Cpu::GetUsagePercent(500) << std::endl;
    std::cout << std::endl;

    auto gpu = Hardware::DisplayAdapter::ListAll();
    if (!gpu.empty())
    {
        std::cout << "[GPU NAME]:" << gpu[0].DescribeName() << std::endl;
        std::cout << "[GPU VIDEO MEMORY]:" << UnitConversion::CapacityKUnitCast(gpu[0].VideoMemory()).ToCapacityString() << std::endl;
        std::cout << "[GPU SYSTEM MEMORY]:" << UnitConversion::CapacityKUnitCast(gpu[0].SystemMemory()).ToCapacityString() << std::endl;
        std::cout << "[GPU SHARED MEMORY]:" << UnitConversion::CapacityKUnitCast(gpu[0].SharedMemory()).ToCapacityString() << std::endl;
        std::cout << std::endl;
    }

    auto adapters = NetAdapter::ListAll();

    for (const auto& adapter : adapters)
    {
        std::stringstream v4AddressBuffer;
        for (const auto& address : adapter.GetIPV4Address())
        {
            v4AddressBuffer << "<" << address.Address() << "|" << address.Mask() << ">";
        }

        std::stringstream v4GatewayBuffer;
        for (const auto& address : adapter.GetIPV4Gateway())
        {
            v4GatewayBuffer << "<" << address << ">";
        }

        std::stringstream v6AddressBuffer;
        for (const auto& address : adapter.GetIPV6Address())
        {
            v6AddressBuffer << "<" << address.Address() << "|" << address.PrefixLength() << ">";
        }

        std::stringstream v6GatewayBuffer;
        for (const auto& address : adapter.GetIPV6Gateway())
        {
            v6GatewayBuffer << "<" << address << ">";
        }
        std::cout << "[NIC ID]:" << adapter.GetId() << std::endl;
#ifdef _WIN32
        std::cout << "[NIC DISPLAY_NAME]:" << CharsetUtils::UTF8ToANSI(adapter.GetDisplayName()) << std::endl;
        std::cout << "[NIC DEVICE_NAME]:" << CharsetUtils::UTF8ToANSI(adapter.GetDeviceName()) << std::endl;
#else
        std::cout << "[NIC DISPLAY_NAME]:" << adapter.GetDisplayName() << std::endl;
        std::cout << "[NIC DEVICE_NAME]:" << adapter.GetDeviceName() << std::endl;
#endif
        std::cout << "[NIC MAC]:" << adapter.GetMac(":") << std::endl;
        std::cout << "[NIC TYPE]:" << NetAdapter::TypeName(adapter.GetType()) << std::endl;
#ifdef _WIN32
        std::cout << "[NIC ENABLE]:" << std::boolalpha << adapter.IsEnable() << std::endl;
#endif

        std::cout << "[NIC CONNECTED]:" << std::boolalpha << adapter.IsConnected() << std::endl;
        std::cout << "[NIC VIRTUAL]:" << std::boolalpha << adapter.IsVirtual() << std::endl;
        std::cout << "[NIC IPV4]:" << v4AddressBuffer.str() << std::endl;
        std::cout << "[NIC GATEWAYV4]:" << v4GatewayBuffer.str() << std::endl;
        std::cout << "[NIC IPV6]:" << v6AddressBuffer.str() << std::endl;
        std::cout << "[NIC GATEWAYV6]:" << v6GatewayBuffer.str() << std::endl;
        std::cout << "[NIC SPEED]:" << UnitConversion::BitrateUnitCast(adapter.GetSpeed()).ToBitrateString() << std::endl;
        std::cout << std::endl;
    }

    auto storages = Hardware::Storage::ListAll();
    for (const auto& storage : storages)
    {
        std::cout << "[STORAGE PATH]:" << storage.Path() << std::endl;
#ifdef _WIN32
        std::cout << "[STORAGE NAME]:" << CharsetUtils::UTF8ToANSI(storage.Name()) << std::endl;
#else
        std::cout << "[STORAGE NAME]:" << storage.Name() << std::endl;
#endif
        std::cout << "[STORAGE TYPE]:" << Hardware::Storage::TypeName(storage.Type()) << std::endl;
        std::cout << "[STORAGE FILESYSTEM]:" << storage.FileSystem() << std::endl;
        std::cout << "[STORAGE TOTAL]:" << UnitConversion::CapacityKUnitCast(storage.TotalCapacity()).ToCapacityString() << std::endl;
        std::cout << "[STORAGE AVAILABLE]:" << UnitConversion::CapacityKUnitCast(storage.AvailableCapacity()).ToCapacityString() << std::endl;
        std::cout << "[STORAGE FREE]:" << UnitConversion::CapacityKUnitCast(storage.FreeCapacity()).ToCapacityString() << std::endl;
        std::cout << "[STORAGE LOCATION]:" << storage.Location() << std::endl;
        std::cout << std::endl;
    }

    auto disks = Hardware::HardDisk::ListAll();
    for (const auto& disk : disks)
    {
        std::cout << "[DISK MODEL]:" << disk.Model() << std::endl;
        std::cout << "[DISK SERIALNUMBER]:" << disk.SerialNumber() << std::endl;
        std::cout << "[DISK CAPACITY]:" << UnitConversion::CapacityKUnitCast(disk.Capacity()).ToCapacityString() << std::endl;
        std::cout << "[DISK BUSTYPE]:" << Hardware::HardDisk::BusTypeName(disk.BusType()) << std::endl;
        std::cout << "[DISK SSD]:" << std::boolalpha << disk.IsSSD() << std::endl;
        std::cout << "[DISK REMOVABLE]:" << std::boolalpha << disk.IsRemovable() << std::endl;
        std::cout << "[DISK LOCATION]:" << disk.Location() << std::endl;
        std::cout << std::endl;
    }

    auto monitors = Hardware::Monitor::ListAll();
    for (const auto& monitor : monitors)
    {
        std::cout << "[MONITOR ID]:" << monitor.Id() << std::endl;
        std::cout << "[MONITOR DEVICE PATH]:" << monitor.DevicePath() << std::endl;
        std::cout << "[MONITOR MODEL]:" << monitor.Model() << std::endl;
        std::cout << "[MONITOR MANUFACTURER]:" << monitor.Manufacturer() << std::endl;
        std::cout << "[MONITOR PRODUCT CODE]:" << monitor.ProductCode() << std::endl;
        std::cout << "[MONITOR SERIALNUMBER]:" << monitor.SerialNumber() << std::endl;
        std::cout << "[MONITOR PHYSICAL WIDTH]:" << monitor.PhysicalWidth() << std::endl;
        std::cout << "[MONITOR PHYSICAL HEIGHT]:" << monitor.PhysicalHeight() << std::endl;
        std::cout << std::endl;
    }
    auto processes = Process::ListAll();
    for (const auto& item : processes)
    {
        std::cout << "[PROCESS " << item.Id() << "]:[" << item.Name() << "] [" << item.ExePathString() << "] ["
                  << FormatSystemClockTime(item.CreateTime(), "%Y-%m-%d:%H:%M:%S") << "]" << std::endl;
    }
    std::cout << std::endl;
#ifdef __linux__
    auto x11Environment = Environment::GetActiveX11Environment();
    std::cout << "[X11]" << std::endl;
    if (x11Environment.has_value())
    {
        for (auto& [key, value] : x11Environment.value())
        {
            std::cout << "[" << key << "]:[" << value << "]" << std::endl;
        }
    }
    std::cout << std::endl;
#endif
    {
        /*
        SystemNotify notify;
        notify.AddStorageChangeCallback([](const std::string& path, SystemNotify::StorageAction action)
                                        { std::cout << "StorageChangeCallback:" << path << " " << static_cast<int>(action) << std::endl; });
        notify.AddDeviceChangeCallback(
            [](const std::vector<std::string>& devicePaths, SystemNotify::DeviceAction action)
            {
                for (const auto& path : devicePaths)
                {
                    std::cout << "DeviceChangeCallback:" << path << " " << static_cast<int>(action) << std::endl;
                }
            }
        );
        notify.Start();
        getchar();
        notify.Stop();
        getchar();
        */
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(TEST, TEST)
{
    zeus::ThreadPool pool(6, false);
    for (auto i = 0; i < 100000; i++)
    {
        pool.CommitTask([]() {});
    }
}