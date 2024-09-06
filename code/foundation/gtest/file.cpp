#include <filesystem>
#include <algorithm>
#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/file/file_utils.h>
#include <zeus/foundation/file/kv_file_utils.h>
#include <zeus/foundation/file/ini_file_utils.h>
#include <zeus/foundation/file/file_utils.h>
#include <zeus/foundation/file/backup_file.h>
#include <zeus/foundation/file/file_wrapper.h>
#include <zeus/foundation/system/win/file_attributes.h>
#include <zeus/foundation/security/win/token.h>
#include <zeus/foundation/time/time.h>

namespace fs = std::filesystem;
using namespace std;
using namespace zeus;

namespace
{
std::map<fs::path, size_t> RandomFiles(const fs::path& dir, size_t count)
{
    fs::create_directories(dir);
    std::map<std::filesystem::path, size_t> files;
    for (size_t i = 0; i < count; ++i)
    {
        auto   filepath = dir / RandWord();
        size_t size     = RandUint32(1024 * 10, 1024 * 1024 * 10);
        files.emplace(filepath, size);
        std::ofstream file(filepath, std::ios::binary);
        file.seekp(size - 1);
        file.write("c", 1);
    }
    return files;
}

bool FileEQ(const fs::path& file1, const fs::path& file2)
{
    ifstream f1(file1, ios::binary);
    ifstream f2(file2, ios::binary);
    if (!f1.is_open() || !f2.is_open())
    {
        return false;
    }
    return FileEqual(f1, f2).value() && FileEqual(file1, file2).value();
}
} // namespace

TEST(file, kv)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"cpuinfo_kv.txt");
    EXPECT_EQ("0", GetKVFileValue(filename, "apicid\t\t", ": "));
    EXPECT_EQ("0", GetKVFileValue(filename, "core id\t\t", ": "));
    std::vector<std::string> apicids;
    for (auto i = 0; i < 20; ++i)
    {
        apicids.emplace_back(std::to_string(i));
    }
    std::vector<std::string> coreids;
    for (auto i = 0; i < 10; ++i)
    {
        coreids.emplace_back(std::to_string(i));
        coreids.emplace_back(std::to_string(i));
    }

    EXPECT_EQ(apicids, GetKVFileValues(filename, "apicid\t\t", ": "));
    EXPECT_EQ(coreids, GetKVFileValues(filename, "core id\t\t", ": "));

    EXPECT_EQ(26 * 20, GetKVFileData(filename, ": ")->size());
    EXPECT_EQ(27 * 20, GetKVFileData(filename, ":")->size());

    EXPECT_EQ(26, GetKVFileUniqueData(filename, ": ")->size());
    EXPECT_EQ(27, GetKVFileUniqueData(filename, ":")->size());

    EXPECT_EQ(2 * 20, PickKVFileData(filename, {"processor\t", "vendor_id\t"}, ": ")->size());
    EXPECT_EQ(2, PickKVFileUniqueData(filename, {"processor\t", "apicid\t\t"}, ":")->size());
}

TEST(file, ini)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"ini_test.txt");
    EXPECT_EQ("310473107007619072", GetIniFileValue(filename, "INI_SECTION_4", "key_twice"));
    std::vector<std::string> data {"310473107007619072", "310473107007619075"};
    EXPECT_EQ(data, GetIniFileValues(filename, "INI_SECTION_4", "key_twice"));
    std::set<std::string> sections {"INI_SECTION_4", "INI_SECTION_3", "INI_SECTION_1", "INI_SECTION_2"};
    EXPECT_EQ(sections, GetIniFileSections(filename));
    std::multimap<std::string, std::string> adminData {
        {"ini_section_1_key_1",   "1345adef146372"},
        {"ini_section_1_key_2", "1345adef14637245"}
    };
    EXPECT_EQ(adminData, GetIniFileData(filename, "INI_SECTION_1"));
    std::multimap<std::string, std::string> INI_SECTION_4Data {
        {"key_twice", "310473107007619072"},
        {"key_twice", "310473107007619075"}
    };
    EXPECT_EQ(INI_SECTION_4Data, GetIniFileData(filename, "INI_SECTION_4"));
    std::map<std::string, std::string> INI_SECTION_4Data2 {
        {"key_twice", "310473107007619072"}
    };
    EXPECT_EQ(INI_SECTION_4Data2, GetIniFileUniqueData(filename, "INI_SECTION_4"));
}

TEST(file, iniTrim)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"com.abc.app.desktop");
    EXPECT_EQ("com.abc.app", GetIniFileValue(filename, "Desktop Entry", "Name"));
    EXPECT_EQ(u8"应用名abc中文", GetIniFileValue(filename, "Desktop Entry", "Name[zh_CN]"));
}

TEST(BackupFile, base)
{
    fs::path filename = zeus::CurrentExe::GetAppPath();
    auto     tempdir  = filename.parent_path() / "backuptemp";
    fs::remove_all(tempdir);
    fs::create_directories(tempdir);
    BackupFile back(tempdir / "1");
    back.AddBackupFile((tempdir / "2"), std::numeric_limits<int32_t>::min());
    back.AddBackupFile((tempdir / "3"), 3);
    EXPECT_TRUE(back.SetContent(RandWord()).has_value());
    EXPECT_TRUE(FileEqual(tempdir / "1", tempdir / "2").value());
    EXPECT_TRUE(FileEqual(tempdir / "1", tempdir / "3").value());
    EXPECT_TRUE(FileEqual(tempdir / "2", tempdir / "3").value());
    auto text = RandString(100);
    {
        ofstream f(tempdir / "1");
        f << text;
    }
    EXPECT_FALSE(FileEqual(tempdir / "1", tempdir / "2").value());
    EXPECT_FALSE(FileEqual(tempdir / "1", tempdir / "3").value());
    EXPECT_TRUE(FileEqual(tempdir / "2", tempdir / "3").value());
    auto data = back.GetContent();
    EXPECT_TRUE(data.has_value());
    EXPECT_EQ(text, data.value());
}

TEST(file, Compare)
{
    fs::path filename = zeus::CurrentExe::GetAppPath();
    auto     tempdir  = filename.parent_path() / "filetemp";
    fs::remove_all(tempdir);
    fs::create_directories(tempdir);

    auto path1 = tempdir / "1";
    auto path2 = tempdir / "2";

    EXPECT_FALSE(FileEqual(path1, path2).has_value());
    {
        ofstream f2(path2);
    }
    EXPECT_FALSE(FileEqual(path1, path2).has_value());
    fs::remove_all(tempdir);
    fs::create_directories(tempdir);
    {
        ofstream f1(path1);
    }
    EXPECT_FALSE(FileEqual(path1, path2).has_value());
    fs::remove_all(tempdir);
    fs::create_directories(tempdir);
    {
        ofstream f1(path1);

        ofstream f2(path2);
    }
    EXPECT_TRUE(FileEQ(path1, path2));
    auto data = RandString(100);
    {
        ofstream f1(path1);
        ofstream f2(path2);
        for (uint64_t index = 0; index <= (std::mega::num * 13 / 100) + 17; ++index)
        {
            f1 << data;
            f2 << data;
        }
    }
    EXPECT_TRUE(FileEQ(path1, path2));
    data = RandString(100);
    {
        ofstream f1(path1);
        f1 << data;
        ofstream f2(path2);
        f2 << data;
        f2 << data;
    }
    EXPECT_FALSE(FileEQ(path1, path2));
    data = RandString(100);
    {
        ofstream f1(path1);
        f1 << data;
        ofstream f2(path2);
        f2 << data;
        f1 << data;
    }
    EXPECT_FALSE(FileEQ(path1, path2));
    {
        ofstream f1(path1);
        f1 << data;
        ofstream f2(path2);
        data[0] = data[0] + 1;
        f2 << data;
    }
    EXPECT_FALSE(FileEQ(path1, path2));
}

TEST(file, Enumerate)
{
#ifdef _WIN32
    ASSERT_TRUE(zeus::WinToken::CurrentProcessHasAdmin().value());
#endif
    fs::path filename = zeus::CurrentExe::GetAppDir();
    auto     main     = filename / "dirTemp";
    auto     subDir   = main / RandWord() / RandWord();
    fs::remove_all(main);

    auto               mainfiles     = RandomFiles(main, 10);
    auto               subFiles      = RandomFiles(subDir, 15);
    size_t             mainTotalSize = 0;
    size_t             subTotalSize  = 0;
    std::set<fs::path> mainFilesSet;
    std::set<fs::path> subFilesSet;
    std::set<fs::path> mainFilesSymbolSet;
    std::set<fs::path> subFilesSymbolSet;
    for (const auto& [path, size] : mainfiles)
    {
        mainTotalSize += size;
        auto sym = fs::u8path(path.u8string() + "_sym");
        fs::create_symlink(path, sym);
        mainFilesSet.emplace(path);
        mainFilesSymbolSet.emplace(sym);
    }
    for (const auto [path, size] : subFiles)
    {
        subTotalSize += size;
        auto sym = fs::u8path(path.u8string() + "_sym");
        fs::create_symlink(path, sym);
        subFilesSet.emplace(path);
        subFilesSymbolSet.emplace(sym);
    }
    auto dirSym = fs::u8path(subDir.u8string() + "_sym");
    fs::create_directory_symlink(subDir, dirSym);

    std::set<fs::path> subDirSymbolSet;
    for (const auto& item : subFilesSet)
    {
        subDirSymbolSet.emplace(dirSym / item.filename());
    }
    std::set<fs::path> subDirSymbolSymbolSet;
    for (const auto& item : subFilesSymbolSet)
    {
        subDirSymbolSymbolSet.emplace(dirSym / item.filename());
    }

    EXPECT_EQ(mainTotalSize, GetDirectoryRegularFileSize(main, false, false, false).value());
    EXPECT_EQ(mainTotalSize + subTotalSize, GetDirectoryRegularFileSize(main, true, false, false).value());
    EXPECT_EQ(mainTotalSize * 2, GetDirectoryRegularFileSize(main, false, false, true).value());
    EXPECT_EQ((mainTotalSize + subTotalSize) * 2, GetDirectoryRegularFileSize(main, true, false, true).value());

    EXPECT_EQ(mainTotalSize * 2, GetDirectoryRegularFileSize(main, false, true, true).value());
    EXPECT_EQ(mainTotalSize + subTotalSize * 2, GetDirectoryRegularFileSize(main, true, true, false).value());
    EXPECT_EQ(mainTotalSize * 2 + subTotalSize * 4, GetDirectoryRegularFileSize(main, true, true, true).value());

    auto regularfiles = GetDirectoryRegularFiles(main, false, false, false).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(mainFilesSet.begin(), mainFilesSet.end(), regularfiles.begin(), regularfiles.end()));
    EXPECT_EQ(mainFilesSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, true, false, false).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_EQ(mainFilesSet.size() + subFilesSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, false, false, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + mainFilesSymbolSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, true, false, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSymbolSet.begin(), subFilesSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + subFilesSet.size() + mainFilesSymbolSet.size() + subFilesSymbolSet.size(), regularfiles.size());

    regularfiles = GetDirectoryRegularFiles(main, false, true, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + mainFilesSymbolSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, true, true, false).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subDirSymbolSet.begin(), subDirSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + subFilesSet.size() + subDirSymbolSet.size(), regularfiles.size());

    regularfiles = GetDirectoryRegularFiles(main, true, true, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSymbolSet.begin(), subFilesSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subDirSymbolSet.begin(), subDirSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subDirSymbolSymbolSet.begin(), subDirSymbolSymbolSet.end()));
    EXPECT_EQ(
        mainFilesSet.size() + subFilesSet.size() + mainFilesSymbolSet.size() + subFilesSymbolSet.size() + subDirSymbolSet.size() +
            subDirSymbolSymbolSet.size(),
        regularfiles.size()
    );
}

TEST(file, Filter)
{
#ifdef _WIN32
    ASSERT_TRUE(zeus::WinToken::CurrentProcessHasAdmin().value());
#endif
    auto main   = zeus::CurrentExe::GetAppDir() / "dirTemp";
    auto subDir = main / RandWord() / RandWord();
    fs::remove_all(main);

    auto               mainfiles     = RandomFiles(main, 10);
    auto               subFiles      = RandomFiles(subDir, 15);
    size_t             mainTotalSize = 0;
    size_t             subTotalSize  = 0;
    std::set<fs::path> mainFilesSet;
    std::set<fs::path> subFilesSet;
    std::set<fs::path> mainFilesSymbolSet;
    std::set<fs::path> subFilesSymbolSet;
    size_t             index = 0;
    for (const auto& [path, size] : mainfiles)
    {
        ++index;
        mainTotalSize += size;
        auto newPath = path;
        if (0 == index % 2)
        {
            newPath.replace_extension("png");
            fs::rename(path, newPath);
            mainFilesSet.emplace(newPath);
        }
        auto sym = newPath.parent_path() / (newPath.stem().u8string() + "_sym");
        if (0 == index % 2)
        {
            sym.replace_extension("png");
            mainFilesSymbolSet.emplace(sym);
        }
        fs::create_symlink(newPath, sym);
    }
    for (const auto [path, size] : subFiles)
    {
        ++index;
        subTotalSize += size;
        auto newPath = path;
        if (0 == index % 2)
        {
            newPath.replace_extension("png");
            fs::rename(path, newPath);
            subFilesSet.emplace(newPath);
        }
        auto sym = newPath.parent_path() / (newPath.stem().u8string() + "_sym");
        if (0 == index % 2)
        {
            sym.replace_extension("png");
            subFilesSymbolSet.emplace(sym);
        }
        fs::create_symlink(newPath, sym);
    }
    auto dirSym = fs::u8path(subDir.u8string() + "_sym");
    fs::create_directory_symlink(subDir, dirSym);

    std::set<fs::path> subDirSymbolSet;
    for (const auto& item : subFilesSet)
    {
        subDirSymbolSet.emplace(dirSym / item.filename());
    }
    std::set<fs::path> subDirSymbolSymbolSet;
    for (const auto& item : subFilesSymbolSet)
    {
        subDirSymbolSymbolSet.emplace(dirSym / item.filename());
    }

    auto regularfiles = GetDirectoryRegularFiles(main, "png", false, false, false).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(mainFilesSet.begin(), mainFilesSet.end(), regularfiles.begin(), regularfiles.end()));
    EXPECT_EQ(mainFilesSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, "png", true, false, false).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_EQ(mainFilesSet.size() + subFilesSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, "png", false, false, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + mainFilesSymbolSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, "png", true, false, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSymbolSet.begin(), subFilesSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + subFilesSet.size() + mainFilesSymbolSet.size() + subFilesSymbolSet.size(), regularfiles.size());

    regularfiles = GetDirectoryRegularFiles(main, "png", false, true, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + mainFilesSymbolSet.size(), regularfiles.size());
    regularfiles = GetDirectoryRegularFiles(main, "png", true, true, false).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subDirSymbolSet.begin(), subDirSymbolSet.end()));
    EXPECT_EQ(mainFilesSet.size() + subFilesSet.size() + subDirSymbolSet.size(), regularfiles.size());

    regularfiles = GetDirectoryRegularFiles(main, "png", true, true, true).value();
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSet.begin(), mainFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSet.begin(), subFilesSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), mainFilesSymbolSet.begin(), mainFilesSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subFilesSymbolSet.begin(), subFilesSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subDirSymbolSet.begin(), subDirSymbolSet.end()));
    EXPECT_TRUE(std::includes(regularfiles.begin(), regularfiles.end(), subDirSymbolSymbolSet.begin(), subDirSymbolSymbolSet.end()));
    EXPECT_EQ(
        mainFilesSet.size() + subFilesSet.size() + mainFilesSymbolSet.size() + subFilesSymbolSet.size() + subDirSymbolSet.size() +
            subDirSymbolSymbolSet.size(),
        regularfiles.size()
    );
}

TEST(file, wrapper)
{
    const auto kRepeatMemoryCheck = [](std::string_view expect, size_t count, const void* data)
    {
        auto offset = reinterpret_cast<const uint8_t*>(data);
        for (size_t index = 0; index < count; ++index)
        {
            EXPECT_EQ(0, memcmp(offset, expect.data(), expect.size()));
            offset += expect.size();
        }
    };

    const auto kRingMemoryCheck = [](std::string_view expect, size_t offset, const void* data)
    {
        EXPECT_EQ(0, memcmp(data, expect.data() + offset, expect.size() - offset));
        EXPECT_EQ(0, memcmp(reinterpret_cast<const uint8_t*>(data) + expect.size() - offset, expect.data(), offset));
    };

#ifdef _WIN32
    ASSERT_TRUE(zeus::WinToken::CurrentProcessHasAdmin().value());
#endif
    auto dir = zeus::CurrentExe::GetAppDir() / "fileTemp";
    fs::remove_all(dir);
    fs::create_directories(dir);
    auto filePath = dir / "test.txt";
    const auto kTestData = zeus::Repeat(u8"Hello, t665hFileWrapperd66h6!撒打发让人呵呵局域里突然特尔u有萨达，。，、【】、=00-", 1024);
    const size_t kRepeatCount         = 100;
    const size_t kTruncateRepeatCount = 108;
    {
        ASSERT_FALSE(FileWrapper::Open(filePath, FileWrapper::OpenMode::kRead).has_value());
    }
    // 创建并写入数据
    {
        auto fileWrapper = FileWrapper::OpenOrCreate(filePath, FileWrapper::OpenMode::kWrite).value();
        ASSERT_TRUE(fileWrapper);
        EXPECT_EQ(kTestData.size(), fileWrapper.Write(kTestData.data(), kTestData.size()).value());
        EXPECT_EQ(kTestData.size() * kRepeatCount, fileWrapper.Write(zeus::Repeat(kTestData, kRepeatCount)).value());
    }

    // 读取数据并验证
    {
        auto fileWrapper = FileWrapper::Open(filePath, FileWrapper::OpenMode::kRead).value();
        ASSERT_TRUE(fileWrapper);
        EXPECT_EQ(kTestData.size() * (1 + kRepeatCount), fileWrapper.FileSize().value());
        auto readStringResult = fileWrapper.ReadString(kTestData.size());
        ASSERT_TRUE(readStringResult.has_value());
        kRepeatMemoryCheck(kTestData, 1, readStringResult.value().data());
        auto readResult = fileWrapper.Read(kTestData.size() * (kRepeatCount + 1));
        ASSERT_TRUE(readResult.has_value());
        kRepeatMemoryCheck(kTestData, kRepeatCount, readResult.value().data());
        EXPECT_EQ(kTestData.size() * (1 + kRepeatCount), fileWrapper.FileSize().value());
    }

    // 读取数据并验证
    {
        auto fileWrapper = FileWrapper::OpenOrCreate(filePath, FileWrapper::OpenMode::kReadWrite).value();
        ASSERT_TRUE(fileWrapper);
        EXPECT_EQ(kTestData.size() * (1 + kRepeatCount), fileWrapper.FileSize().value());
        auto readStringResult = fileWrapper.ReadString(kTestData.size());
        ASSERT_TRUE(readStringResult.has_value());
        kRepeatMemoryCheck(kTestData, 1, readStringResult.value().data());
        auto readResult = fileWrapper.Read(kTestData.size() * (kRepeatCount + 1));
        ASSERT_TRUE(readResult.has_value());
        kRepeatMemoryCheck(kTestData, kRepeatCount, readResult.value().data());
        EXPECT_EQ(kTestData.size() * (1 + kRepeatCount), fileWrapper.FileSize().value());
    }
    // 截断文件并写入数据
    {
        auto fileWrapper = FileWrapper::Truncate(filePath, FileWrapper::OpenMode::kReadWrite, true).value();
        ASSERT_TRUE(fileWrapper);
        EXPECT_EQ(kTestData.size(), fileWrapper.Write(kTestData.data(), kTestData.size()).value());
        EXPECT_EQ(kTestData.size() * kTruncateRepeatCount, fileWrapper.Write(zeus::Repeat(kTestData, kTruncateRepeatCount)).value());
        EXPECT_TRUE(fileWrapper.Flush().has_value());
    }
    // 读取数据并验证
    {
        auto fileWrapper = FileWrapper::Open(filePath, FileWrapper::OpenMode::kReadWrite).value();
        ASSERT_TRUE(fileWrapper);
        EXPECT_EQ(kTestData.size() * (1 + kTruncateRepeatCount), fileWrapper.FileSize().value());
        auto readStringResult = fileWrapper.ReadString(kTestData.size());
        ASSERT_TRUE(readStringResult.has_value());
        kRepeatMemoryCheck(kTestData, 1, readStringResult.value().data());
        auto readResult = fileWrapper.Read(kTestData.size() * kTruncateRepeatCount);
        ASSERT_TRUE(readResult.has_value());
        kRepeatMemoryCheck(kTestData, kTruncateRepeatCount, readResult.value().data());
        EXPECT_EQ(kTestData.size() * (1 + kTruncateRepeatCount), fileWrapper.FileSize().value());
    }

    // 读取数据并验证
    {
        auto fileWrapper = FileWrapper::OpenOrCreate(filePath, FileWrapper::OpenMode::kRead).value();
        ASSERT_TRUE(fileWrapper);
        EXPECT_EQ(kTestData.size() * (1 + kTruncateRepeatCount), fileWrapper.FileSize().value());
        auto readStringResult = fileWrapper.ReadString(kTestData.size());
        ASSERT_TRUE(readStringResult.has_value());
        kRepeatMemoryCheck(kTestData, 1, readStringResult.value().data());
        auto readResult = fileWrapper.Read(kTestData.size() * kTruncateRepeatCount);
        ASSERT_TRUE(readResult.has_value());
        kRepeatMemoryCheck(kTestData, kTruncateRepeatCount, readResult.value().data());
        EXPECT_EQ(kTestData.size() * (1 + kTruncateRepeatCount), fileWrapper.FileSize().value());
    }

    // 读取数据并验证Seek和Tell
    {
        auto fileWrapper = FileWrapper::Open(filePath, FileWrapper::OpenMode::kRead).value();
        ASSERT_TRUE(fileWrapper);
        auto count = RandUint32(100, 200);
        for (size_t index = 0; index < count; ++index)
        {
            auto offset           = RandUint32(1, kTestData.size() * (kTruncateRepeatCount));
            auto readStringResult = fileWrapper.ReadString(kTestData.size(), offset, FileWrapper::OffsetType::kBegin);
            ASSERT_TRUE(readStringResult.has_value());
            kRingMemoryCheck(kTestData, offset % kTestData.size(), readStringResult.value().data());
            auto tellResult = fileWrapper.Tell();
            ASSERT_TRUE(tellResult.has_value());
            EXPECT_EQ(tellResult.value(), offset + kTestData.size());
            fileWrapper.Seek(-static_cast<int64_t>(kTestData.size()), FileWrapper::OffsetType::kCurrent);
            tellResult = fileWrapper.Tell();
            ASSERT_TRUE(tellResult.has_value());
            EXPECT_EQ(tellResult.value(), offset);
            auto readResult = fileWrapper.Read(kTestData.size());
            ASSERT_TRUE(readResult.has_value());
            kRingMemoryCheck(kTestData, offset % kTestData.size(), readResult.value().data());
            tellResult = fileWrapper.Tell();
            ASSERT_TRUE(tellResult.has_value());
            EXPECT_EQ(tellResult.value(), offset + kTestData.size());
        }
    }
    {
        ASSERT_FALSE(FileWrapper::Create(filePath, FileWrapper::OpenMode::kReadWrite).has_value());
    }
    {
        Sleep(std::chrono::seconds(10));
        auto fileWrapper = FileWrapper::Open(filePath, FileWrapper::OpenMode::kRead).value();
        ASSERT_TRUE(fileWrapper);
        auto createTimeResult = fileWrapper.CreateTime();
        ASSERT_TRUE(createTimeResult.has_value());
        auto createTime = createTimeResult.value();
        auto now        = std::chrono::system_clock::now();
        EXPECT_LE(std::chrono::seconds(10), now - createTime);
        auto lastAccessTimeResult = fileWrapper.LastAccessTime();
        ASSERT_TRUE(lastAccessTimeResult.has_value());
        auto lastAccessTime = lastAccessTimeResult.value();
        EXPECT_LE(std::chrono::seconds(1), now - lastAccessTime);
        auto lastWriteTimeResult = fileWrapper.LastWriteTime();
        ASSERT_TRUE(lastWriteTimeResult.has_value());
        auto lastWriteTime = lastWriteTimeResult.value();
        EXPECT_LE(std::chrono::seconds(10), now - lastWriteTime);
        auto lastChangeTimeResult = fileWrapper.LastChangeTime();
        ASSERT_TRUE(lastChangeTimeResult.has_value());
        auto lastChangeTime = lastChangeTimeResult.value();
        EXPECT_LE(std::chrono::seconds(10), now - lastChangeTime);
    }
    {
        auto symPath = dir / "test_sym.txt";
        fs::create_symlink(filePath, symPath);
        auto fileWrapper = FileWrapper::OpenSymbolLink(symPath, FileWrapper::OpenMode::kRead).value();
        ASSERT_TRUE(fileWrapper);
#ifdef _WIN32
        EXPECT_EQ(0, fileWrapper.FileSize().value());
#endif
#ifdef __linux__
        EXPECT_EQ(filePath.u8string().size(), fileWrapper.FileSize().value());
#endif
    }
}

TEST(file, CreateDirectory)
{
    EXPECT_FALSE(CreateWriteableDirectory(zeus::CurrentExe::GetAppPath()).has_value());
    EXPECT_TRUE(CreateWriteableDirectory(zeus::CurrentExe::GetAppDir()).has_value());
    auto dir = zeus::CurrentExe::GetAppDir() / "dirTemp";
    fs::remove_all(dir);
    std::error_code ec;
    ASSERT_FALSE(fs::exists(dir, ec));
    for (size_t index = 0; index < 5; ++index)
    {
        dir /= RandString(5);
        ASSERT_FALSE(fs::exists(dir, ec));
    }
    ASSERT_FALSE(fs::exists(dir, ec));
    auto ret = CreateWriteableDirectory(dir);
    ASSERT_TRUE(ret.has_value());
    ASSERT_TRUE(fs::exists(dir, ec));
#ifdef _WIN32
    EXPECT_FALSE(CreateWriteableDirectory(fs::u8path("10:/test/test")).has_value());
#endif
}
