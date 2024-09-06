#include <atomic>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/resource/auto_raii.h>
#include <zeus/foundation/resource/auto_release.h>
#include <zeus/foundation/resource/file_mapping.h>
#include <zeus/foundation/ipc/memory_mapping.h>

using namespace zeus;
namespace fs = std::filesystem;

TEST(AutoRaii, base)
{
    size_t count = 0;
    {
        AutoRaii raii(
            [&count]()
            {
                count++;
                EXPECT_EQ(1, count);
            },
            [&count]()
            {
                count--;
                EXPECT_EQ(0, count);
            }
        );
        EXPECT_EQ(1, count);
    }
    EXPECT_EQ(0, count);
}
TEST(AutoRaii, move)
{
    size_t count = 0;
    {
        AutoRaii move(
            [&count]()
            {
                count++;
                EXPECT_EQ(1, count);
            },
            [&count]()
            {
                count--;
                EXPECT_EQ(0, count);
            }
        );

        AutoRaii raii(std::move(move));
        EXPECT_EQ(1, count);
    }
    EXPECT_EQ(0, count);
}

TEST(AutoRelease, base)
{
    bool status = false;
    {
        AutoRelease raii([&status]() { status = true; });
    }
    EXPECT_TRUE(status);
}

TEST(AutoRelease, NonCopyable)
{
    int const                                     kSize = 100;
    std::vector<std::unique_ptr<std::atomic_int>> flags {};
    for (int i = 0; i < kSize; ++i)
    {
        flags.emplace_back(std::make_unique<std::atomic_int>(0));
    }
    {
        std::vector<AutoRelease> raiiList {};
        raiiList.reserve(kSize);
        for (int i = 0; i < kSize; ++i)
        {
            raiiList.emplace_back([&flag(*flags.at(i))]() { flag.fetch_add(1); });
        }
        // force reallocate in vector
        ASSERT_EQ(kSize, raiiList.capacity());
        raiiList.emplace_back([] {});
    }
    for (std::unique_ptr<std::atomic_int>& flag : flags)
    {
        EXPECT_EQ(1, flag->load());
    }
}
TEST(AutoRelease, Defer)
{
    int cnt = 1;
    {
        zeus::AutoRelease Defer507 {[&]()
                                    {
                                        ++cnt;
                                    }};
    }
    EXPECT_EQ(2, cnt);
    for (int i = 2; i < 10; ++i)
    {
        {
            zeus::AutoRelease Defer516 {[&]()
                                        {
                                            EXPECT_EQ(i, cnt);
                                            ++cnt;
                                        }};
        }
        EXPECT_EQ(i + 1, cnt);
    }
}

TEST(FileMapping, base)
{
    static constexpr size_t kSize    = 1024 * 1024;
    auto                    tempDir  = fs::temp_directory_path();
    auto                    filePath = tempDir / "FileMapping_test";
    std::error_code         ec;
    {
        auto data = RandString(kSize);
        fs::remove(filePath, ec);
        ASSERT_FALSE(fs::exists(filePath, ec));
        std::ofstream file(filePath, std::ios::binary);
        file.write(data.data(), data.size());
        file.close();
        fs::permissions(filePath, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read, ec);
        {
            auto offset      = RandUint32(MemoryMapping::SystemMemoryAlign() + 1, MemoryMapping::SystemMemoryAlign() * 2 - 1);
            auto fileMapping = FileMapping::Create(filePath, false);
            ASSERT_TRUE(fileMapping.has_value());
            ASSERT_TRUE(fileMapping->MapAll(offset).has_value());
            EXPECT_EQ(fileMapping->Size(), kSize - offset);
            EXPECT_EQ(0, std::memcmp(fileMapping->Data(), data.data() + offset, kSize - offset));
        }
        {
            auto fileMapping = FileMapping::Create(filePath, false);
            ASSERT_TRUE(fileMapping.has_value());
            ASSERT_TRUE(fileMapping->MapAll().has_value());
            EXPECT_EQ(fileMapping->Size(), kSize);
            EXPECT_EQ(0, std::memcmp(fileMapping->Data(), data.data(), kSize));
        }
    }
    {
        auto data = RandString(kSize);
        ASSERT_TRUE(fs::exists(filePath, ec));
        ASSERT_EQ(kSize, fs::file_size(filePath, ec));
        auto offset = RandUint32(MemoryMapping::SystemMemoryAlign() + 1, MemoryMapping::SystemMemoryAlign() * 2 - 1);
        {
            auto fileMapping = FileMapping::Create(filePath, true);
            EXPECT_FALSE(fileMapping.has_value());
            fs::permissions(filePath, fs::perms::owner_all | fs::perms::owner_all | fs::perms::owner_all, ec);
            fileMapping = FileMapping::Create(filePath, true);
            EXPECT_TRUE(fileMapping.has_value());
            ASSERT_TRUE(fileMapping->Map(offset, kSize - offset).has_value());
            ASSERT_TRUE(fileMapping->Map(offset, kSize - offset).has_value());
            EXPECT_EQ(fileMapping->Size(), kSize - offset);
            std::memcpy(fileMapping->Data(), data.data() + offset, kSize - offset);
        }
        auto          buffer = std::make_unique<char[]>(kSize);
        std::ifstream file(filePath, std::ios::binary);
        file.read(buffer.get(), kSize);

        EXPECT_EQ(0, std::memcmp(buffer.get() + offset, data.data() + offset, kSize - offset));
        file.close();
    }
}