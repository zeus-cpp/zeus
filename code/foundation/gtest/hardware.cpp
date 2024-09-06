#include <algorithm>
#include <gtest/gtest.h>
#include <zeus/foundation/hardware/cpu.h>
#include <zeus/foundation/hardware/storage.h>
#include <zeus/foundation/hardware/hard_disk.h>
#include <zeus/foundation/hardware/monitor.h>

TEST(Hardware, CpuPerformance)
{
    for (auto i = 0; i < 10; i++)
    {
        auto usagePercent = zeus::Hardware::Cpu::GetUsagePercent();
        EXPECT_LE(usagePercent, 100);
        EXPECT_GE(usagePercent, 0);
    }
}

TEST(Hardware, CpuCoreCount)
{
    auto cpus = zeus::Hardware::Cpu::ListAll();
    EXPECT_GT(cpus.size(), 0);
}

TEST(Hardware, StorageType)
{
    auto storages = zeus::Hardware::Storage::List(zeus::Hardware::Storage::StorageType::kDisk);
    for (const auto& storage : storages)
    {
        EXPECT_EQ(storage.Type(), zeus::Hardware::Storage::StorageType::kDisk);
    }
}

TEST(Hardware, HardDiskType)
{
    auto disks = zeus::Hardware::HardDisk::ListRemovable(true);
    for (const auto& disk : disks)
    {
        EXPECT_TRUE(disk.IsRemovable());
    }
    disks = zeus::Hardware::HardDisk::ListRemovable(false);
    for (const auto& disk : disks)
    {
        EXPECT_FALSE(disk.IsRemovable());
    }
}

TEST(Hardware, RemovableStorage)
{
    const auto hardDisks = zeus::Hardware::HardDisk::ListAll();
    auto       storages  = zeus::Hardware::Storage::ListRemovableStorage(true);
    for (const auto& storage : storages)
    {
        EXPECT_EQ(zeus::Hardware::Storage::StorageType::kDisk, storage.Type());
        EXPECT_TRUE(std::any_of(
            hardDisks.begin(), hardDisks.end(),
            [&storage](const zeus::Hardware::HardDisk& disk) { return disk.Location() == storage.Location() && disk.IsRemovable(); }
        ));
    }
    storages = zeus::Hardware::Storage::ListRemovableStorage(false);
    for (const auto& storage : storages)
    {
        EXPECT_EQ(zeus::Hardware::Storage::StorageType::kDisk, storage.Type());
        EXPECT_FALSE(std::any_of(
            hardDisks.begin(), hardDisks.end(),
            [&storage](const zeus::Hardware::HardDisk& disk) { return disk.Location() == storage.Location() && disk.IsRemovable(); }
        ));
    }
}

TEST(Hardware, Monitor)
{
    auto infos = zeus::Hardware::Monitor::ListAll();
}