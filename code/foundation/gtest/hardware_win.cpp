#ifdef _WIN32
#include <thread>
#include <algorithm>
#include <Windows.h>
#include <winioctl.h>
#include <gtest/gtest.h>
#include <zeus/foundation/hardware/cpu.h>
#include <zeus/foundation/hardware/storage.h>
#include <zeus/foundation/hardware/hard_disk.h>
#include <zeus/foundation/hardware/display_adapter.h>
#include <zeus/foundation/hardware/screen.h>
#include <zeus/foundation/hardware/base_board.h>
#include <zeus/foundation/hardware/memory.h>
#include <zeus/foundation/hardware/system.h>
#include <zeus/foundation/system/os.h>
#include <zeus/foundation/system/win/registry.h>
#include <zeus/foundation/system/win/wmi.h>
#include <zeus/foundation/hardware/smbios.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/system/win/session.h>

using namespace zeus;
using namespace zeus::Hardware;

TEST(Hardware, CpuInfo)
{
    auto infos = Cpu::ListAll();
    ASSERT_FALSE(infos.empty());
    if (!infos.empty())
    {
        auto processor = zeus::WinRegistry::OpenKey(HKEY_LOCAL_MACHINE, R"(HARDWARE\DESCRIPTION\System\CentralProcessor\)", true);
        auto wmi       = WinWMI::Create();
        ASSERT_TRUE(wmi.has_value());
        auto datas = wmi->Query("SELECT * FROM Win32_Processor").value();
        EXPECT_EQ(datas.size(), infos.size());
        EXPECT_EQ(infos.front().Name(), datas[0]["Name"].get<std::string>());
        EXPECT_EQ(infos.front().PhysicalCoreCount(), datas[0]["NumberOfCores"].get<size_t>());
        EXPECT_EQ(infos.front().LogicalCoreCount(), datas[0]["NumberOfLogicalProcessors"].get<size_t>());
        std::vector<std::string> core = *processor->GetSubKeys();
        EXPECT_EQ(infos.front().LogicalCoreCount(), core.size());
        EXPECT_EQ(infos.front().LogicalCoreCount(), std::thread::hardware_concurrency());
    }
}

TEST(Hardware, Storage)
{
    auto infos = Storage::ListAll();
    auto wmi   = WinWMI::Create();
    ASSERT_TRUE(wmi.has_value());
    auto datas = wmi->Query("SELECT * FROM Win32_LogicalDisk").value();
    EXPECT_LE(infos.size(), datas.size());
    for (auto& data : datas)
    {
        auto name = data["DeviceID"].get<std::string>();
        auto info = std::find_if(infos.begin(), infos.end(), [&name](const Storage& item) { return name == item.Path(); });
        auto type = data["MediaType"].is_null() ? 11 : data["MediaType"].get<size_t>();

        if (type != 11 && type != 12 && type != 0)
        {
            EXPECT_EQ(info->Type(), Storage::StorageType::kFloppy);
        }
        else
        {
            switch (data["DriveType"].get<size_t>())
            {
            case DRIVE_CDROM:
                EXPECT_EQ(Storage::StorageType::kCD, info->Type());
                break;
            case DRIVE_FIXED:
            case DRIVE_REMOVABLE:
                EXPECT_EQ(Storage::StorageType::kDisk, info->Type());
                break;
            case DRIVE_REMOTE:
                EXPECT_EQ(Storage::StorageType::kRemote, info->Type());
                break;
            case DRIVE_RAMDISK:
                EXPECT_EQ(Storage::StorageType::kRAM, info->Type());
                break;
            default:
                EXPECT_TRUE(false);
            }
        }
        if (data.contains("VolumeName"))
        {
            EXPECT_EQ(info->Name(), data["VolumeName"].get<std::string>());
        }

        if (data.contains("FileSystem"))
        {
            EXPECT_EQ(info->FileSystem(), data["FileSystem"].get<std::string>());
        }
        if (data.contains("FreeSpace"))
        {
            auto freeDiff = std::abs(static_cast<int64_t>(info->FreeCapacity()) - std::stoll(data["FreeSpace"].get<std::string>()) / 1024);
            EXPECT_LE(freeDiff, 1000); //小于1Mb是正常的波动
        }
        if (data.contains("Size"))
        {
            EXPECT_EQ(info->TotalCapacity(), std::stoull(data["Size"].get<std::string>()) / 1024);
        }
    }
}

TEST(Hardware, DisplayCard)
{
    auto infos = DisplayAdapter::ListAll();
    if (2 == infos.size() && infos[0].DescribeName() == infos[1].DescribeName() && infos[0].VideoMemory() == infos[1].VideoMemory())
    {
        //win11下莫名其妙出现两个一样的intel显卡
        infos.erase(infos.begin());
    }
    auto wmi = WinWMI::Create();
    ASSERT_TRUE(wmi.has_value());
    auto           datas = wmi->Query("SELECT * FROM Win32_VideoController").value();
    nlohmann::json realDatas;
    for (auto& data : datas)
    {
        if (!data["AdapterRAM"].is_null() && data["AdapterRAM"].get<size_t>())
        {
            realDatas.emplace_back(data);
        }
    }
    EXPECT_EQ(infos.size(), realDatas.size());
    for (auto& data : realDatas)
    {
        auto name = data["Description"].get<std::string>();

        auto iter = std::find_if(infos.begin(), infos.end(), [&name](const DisplayAdapter& item) { return name == item.DescribeName(); });
        EXPECT_TRUE(iter != infos.end());
    }
}

TEST(Hardware, DisplayScreen)
{
    Screen::GetDisplayScreenInfo();
}

TEST(Hardware, HardDisk)
{
    auto msftQuery = WinWMI::Create(R"(root\Microsoft\Windows\Storage)");
    ASSERT_TRUE(msftQuery.has_value());

    auto msftDisks = msftQuery->Query("Select * From MSFT_PhysicalDisk").value();

    auto cimQuery = WinWMI::Create();
    ASSERT_TRUE(cimQuery.has_value());
    auto cimDisks = cimQuery->Query("SELECT * FROM Win32_DiskDrive").value();
    auto infos    = HardDisk::ListAll();
    for (const auto& info : infos)
    {
        for (const auto& disk : msftDisks)
        {
            if (disk["FriendlyName"].get<std::string>() == info.Model())
            {
                EXPECT_EQ(disk["MediaType"].get<size_t>() == 4, info.IsSSD());
                switch (disk["BusType"].get<size_t>())
                {
                case STORAGE_BUS_TYPE::BusTypeUsb:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kUSB, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeAta:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kATA, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeSd:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kSD, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeSas:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kSAS, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeSata:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kSATA, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeScsi:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kSCSI, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeNvme:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kNVMe, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeRAID:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kRAID, info.BusType());
                    break;
                case STORAGE_BUS_TYPE::BusTypeFileBackedVirtual:
                case STORAGE_BUS_TYPE::BusTypeVirtual:
                    EXPECT_EQ(HardDisk::HardDiskBusType::kVirtual, info.BusType());
                    break;
                default:
                    EXPECT_TRUE(false);
                    break;
                }

                if (disk.contains("SerialNumber"))
                {
                    EXPECT_EQ(disk["SerialNumber"].get<std::string>(), info.SerialNumber());
                }
            }
        }

        for (const auto& disk : cimDisks)
        {
            if (disk["Model"].get<std::string>() == info.Model())
            {
                if (disk.contains("SerialNumber"))
                {
                    auto sn = zeus::Trim(disk["SerialNumber"].get<std::string>());
                    EXPECT_EQ(sn, info.SerialNumber());
                }
                EXPECT_EQ(std::stoull(disk["Size"].get<std::string>()) / 1024, info.Capacity());

                if (info.IsRemovable())
                {
                    auto s1 = zeus::ToLowerCopy(disk["MediaType"].get<std::string>());
                    auto s2 = zeus::ToLowerCopy("Removable media");
                    EXPECT_NE(s1.find(s2), std::string::npos);
                }
            }
        }
    }
    EXPECT_GE(infos.size(), 1);
}

TEST(Hardware, BaseBoard)
{
    auto info = BaseBoard::GetBaseBoard();
    auto wmi  = WinWMI::Create();
    ASSERT_TRUE(wmi.has_value());
    auto data = wmi->Query("SELECT * FROM Win32_BaseBoard").value();
    EXPECT_EQ(data[0]["Manufacturer"].get<std::string>(), info.Manufacturer());
    EXPECT_EQ(data[0]["SerialNumber"].get<std::string>(), info.SerialNumber());
    EXPECT_EQ(data[0]["Product"].get<std::string>(), info.Product());
}

TEST(Hardware, Memory)
{
    auto info = Memory::GetMemory(true);
    auto wmi  = WinWMI::Create();
    ASSERT_TRUE(wmi.has_value());
    auto data = wmi->Query("SELECT * FROM Win32_OperatingSystem").value();
    EXPECT_EQ(std::stoull(data[0]["TotalVisibleMemorySize"].get<std::string>()), info.VisiblePhysicalCapacity());
    data = wmi->Query("SELECT * FROM Win32_PhysicalMemory").value();
    EXPECT_EQ(data.size(), info.BankList().size());
    uint64_t size = 0;
    for (auto& item : data)
    {
        size += std::stoull(item["Capacity"].get<std::string>());
    }
    EXPECT_EQ(size / 1024, info.InstalledPhysicalCapacity());
}

TEST(Hardware, SMBIOS)
{
    auto data = SMBIOS::GetSMBIOS(SMBIOS::InfoType::kSMBIOS_BIOS);
    EXPECT_FALSE(data.empty());
    data = SMBIOS::GetSMBIOS(SMBIOS::InfoType::kSMBIOS_SYSTEM);
    EXPECT_FALSE(data.empty());
    data = SMBIOS::GetSMBIOS(SMBIOS::InfoType::kSMBIOS_BASEBOARD);
    EXPECT_FALSE(data.empty());
    data = SMBIOS::GetSMBIOS(SMBIOS::InfoType::kSMBIOS_PROCESSOR);
    EXPECT_FALSE(data.empty());
    data = SMBIOS::GetSMBIOS(SMBIOS::InfoType::kSMBIOS_PROCESSOR_CACHE);
    data = SMBIOS::GetSMBIOS(SMBIOS::InfoType::kSMBIOS_MEMORYDEVICE);
    EXPECT_FALSE(data.empty());
}

TEST(Hardware, System)
{
    auto data = System::GetSystem();
    EXPECT_FALSE(data.Manufacturer().empty());
}

#endif
