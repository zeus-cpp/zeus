#ifdef _WIN32
#include <thread>
#include <future>
#include <fstream>
#include <Windows.h>
#include <CommCtrl.h>
#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/crypt/uuid.h>
#include <zeus/foundation/resource/win/com_init.h>
#include <zeus/foundation/resource/auto_release.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/string/charset_utils.h>
#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/system/environment_variable.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/system/os.h>
#include <zeus/foundation/system/shared_library.h>
#include <zeus/foundation/system/win/system_service.h>
#include <zeus/foundation/system/win/wmi.h>
#include <zeus/foundation/system/win/shell_link.h>
#include <zeus/foundation/system/win/file_manifest.h>
#include <zeus/foundation/system/win/file_digital_sign.h>
#include <zeus/foundation/system/win/file_icon.h>
#include <zeus/foundation/system/win/file_version_info.h>
#include <zeus/foundation/system/win/file_attributes.h>
#include <zeus/foundation/system/win/session.h>
#include <zeus/foundation/system/win/firewall_policy_manager.h>
#include <zeus/foundation/security/win/token.h>
#include <zeus/foundation/sync/event.h>
#include <zeus/foundation/file/file_utils.h>
#include <zeus/foundation/system/environment_variable.h>

using namespace zeus;

namespace fs = std::filesystem;
using namespace std::literals::string_view_literals;

TEST(Com, Init)
{
    auto future = std::async(
        std::launch::async,
        []()
        {
            ComInit init(ComInit::ComType::APARTMENTTHREADED);
            EXPECT_EQ(init.CurrentType(), ComInit::ComType::APARTMENTTHREADED);
            EXPECT_EQ(init.CurrentThreadId(), std::this_thread::get_id());
        }
    );

    future = std::async(
        std::launch::async,
        []()
        {
            ComInit init(ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init.CurrentType(), ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init.CurrentThreadId(), std::this_thread::get_id());
        }
    );
    future = std::async(
        std::launch::async,
        []()
        {
            ComInit init1(ComInit::ComType::MULTITHREADED);
            ComInit init2(ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init1.CurrentType(), ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init1.CurrentThreadId(), std::this_thread::get_id());
            EXPECT_EQ(init2.CurrentType(), ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init2.CurrentThreadId(), std::this_thread::get_id());
        }
    );

    future = std::async(
        std::launch::async,
        []()
        {
            ComInit init1(ComInit::ComType::APARTMENTTHREADED);
            ComInit init2(ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init1.CurrentType(), ComInit::ComType::APARTMENTTHREADED);
            EXPECT_EQ(init1.CurrentThreadId(), std::this_thread::get_id());
            EXPECT_EQ(init2.CurrentType(), ComInit::ComType::APARTMENTTHREADED);
            EXPECT_EQ(init2.CurrentThreadId(), std::this_thread::get_id());
        }
    );

    future = std::async(
        std::launch::async,
        []()
        {
            ComInit init1(ComInit::ComType::MULTITHREADED);
            ComInit init2(ComInit::ComType::APARTMENTTHREADED);
            EXPECT_EQ(init1.CurrentType(), ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init1.CurrentThreadId(), std::this_thread::get_id());
            EXPECT_EQ(init2.CurrentType(), ComInit::ComType::MULTITHREADED);
            EXPECT_EQ(init2.CurrentThreadId(), std::this_thread::get_id());
        }
    );
}

TEST(Wmi, Query)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
}

TEST(Wmi, Thread)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto future = std::async(std::launch::async, [&query]() { EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty()); });
}

TEST(Wmi, ThreadError1)
{
    std::optional<WinWMI> query;

    auto future = std::async(
        std::launch::async,
        [&query]()
        {
            ComInit init(ComInit::ComType::MULTITHREADED);
            query = std::move(WinWMI::Create().value());
            EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
            query.reset();
        }
    );
}

TEST(Wmi, ThreadError2)
{
    std::optional<WinWMI> query;

    auto future = std::async(
        std::launch::async,
        [&query]()
        {
            ComInit init(ComInit::ComType::APARTMENTTHREADED);
            query = std::move(WinWMI::Create().value());
            EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
            query.reset();
        }
    );
}

TEST(Wmi, ThreadError3)
{
    std::optional<zeus::WinWMI> query;
    zeus::Event                 init, select;
    auto                        future = std::async(
        std::launch::async,
        [&query, &init, &select]()
        {
            query = std::move(WinWMI::Create().value());
            init.Notify();
            select.Wait();
            query.reset();
        }
    );
    init.Wait();
    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
    select.Notify();
}

TEST(Wmi, ThreadError4)
{
    std::optional<zeus::WinWMI> query;
    zeus::Event                 init, select;
    auto                        future = std::async(
        std::launch::async,
        [&query, &init, &select]()
        {
            query = std::move(WinWMI::Create().value());
            init.Notify();
            select.Wait();
            query.reset();
        }
    );
    init.Wait();
    ComInit init1(ComInit::ComType::APARTMENTTHREADED);
    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
    select.Notify();
}

TEST(Wmi, ThreadError5)
{
    ComInit init(ComInit::ComType::APARTMENTTHREADED);
    auto    query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto future = std::async(std::launch::async, [&query]() { EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty()); });
}

TEST(Wmi, ThreadError6)
{
    std::optional<zeus::WinWMI> query;
    zeus::Event                 init, select;
    auto                        future = std::async(
        std::launch::async,
        [&query, &init, &select]()
        {
            query = std::move(WinWMI::Create().value());
            init.Notify();
            select.Wait();
            query.reset();
        }
    );
    init.Wait();
    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
    {
        std::thread([&query, &init, &select]() { EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty()); }).join();
    }
    {
        std::thread(
            [&query, &init, &select]()
            {
                ComInit init1(ComInit::ComType::APARTMENTTHREADED);
                EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
            }
        ).join();
    }
    select.Notify();
}

TEST(Wmi, ThreadError7)
{
    std::optional<zeus::WinWMI> query;
    zeus::Event                 init, select;
    auto                        future = std::async(
        std::launch::async,
        [&query, &init, &select]()
        {
            query = std::move(WinWMI::Create().value());
            init.Notify();
            select.Wait();
            query.reset();
        }
    );
    init.Wait();
    ComInit init1(ComInit::ComType::APARTMENTTHREADED);
    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
    {
        std::thread([&query, &init, &select]() { EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty()); }).join();
    }
    {
        std::thread(
            [&query, &init, &select]()
            {
                ComInit init1(ComInit::ComType::APARTMENTTHREADED);
                EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
            }
        ).join();
    }
    select.Notify();
}

TEST(Wmi, ThreadError8)
{
    ComInit init(ComInit::ComType::APARTMENTTHREADED);
    auto    query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    {
        std::thread([&query]() { EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty()); }).join();
    }
    {
        std::thread(
            [&query]()
            {
                ComInit init1(ComInit::ComType::APARTMENTTHREADED);
                EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
            }
        ).join();
    }
    {
        std::thread([&query]() { EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty()); }).join();
    }
}

TEST(Wmi, MultiThread)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    std::thread threads[5];
    for (int i = 0; i < 5; i++)
    {
        threads[i] = std::thread(
            [&query]()
            {
                for (int i = 0; i < 100; i++)
                {
                    std::unique_ptr<ComInit> init;
                    if (i > 50 && i < 60)
                    {
                        init = std::make_unique<ComInit>(ComInit::ComType::APARTMENTTHREADED);
                    }
                    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
                }
            }
        );
    }
    for (int i = 0; i < 5; i++)
    {
        threads[i].join();
    }
}

TEST(Wmi, MultiThread1)
{
    ComInit init(ComInit::ComType::APARTMENTTHREADED);
    auto    query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    std::thread threads[5];
    for (int i = 0; i < 5; i++)
    {
        threads[i] = std::thread(
            [&query]()
            {
                for (int i = 0; i < 100; i++)
                {
                    std::unique_ptr<ComInit> init;
                    if (i > 50 && i < 60)
                    {
                        init = std::make_unique<ComInit>(ComInit::ComType::APARTMENTTHREADED);
                    }
                    EXPECT_FALSE(query->Query("SELECT * FROM Win32_QuickFixEngineering")->empty());
                }
            }
        );
    }
    for (int i = 0; i < 5; i++)
    {
        threads[i].join();
    }
}

#if _WIN32_WINNT >= 0x0600
TEST(Wmi, QueryNamespace)
{
    if (OS::IsWin7OrGreater() && !zeus::OS::IsWinServer())
    {
        auto query = WinWMI::Create(R"(root\Microsoft\Windows\Storage)");
        ASSERT_TRUE(query.has_value());
        EXPECT_FALSE(query->Query("Select FriendlyName, MediaType From MSFT_PhysicalDisk")->empty());
    }
}
#endif

TEST(WinShellLink, base)
{
    fs::path        appDir     = zeus::CurrentExe::GetAppDir() / "symlinkTemp";
    fs::path        linkPath   = appDir / (zeus::Uuid::GenerateRandom().toString() + ".lnk");
    fs::path        targetPath = appDir / zeus::Uuid::GenerateRandom().toString();
    std::error_code code;
    fs::remove_all(appDir, code);
    fs::create_directories(appDir, code);
    zeus::AutoRelease autoClear {[&]()
                                 {
                                     fs::remove(linkPath, code);
                                     fs::remove(targetPath, code);
                                 }};
    const std::string description1 = "zeus.Description";
    const std::string args1        = "zeus.Args";
    const std::string workingDir1  = appDir.u8string();
    const std::string icon1        = R"(C:\Program Files\Common Files\TortoiseOverlays\icons\Ribbon\AddedIcon.ico)";
    const std::string description2 = "zeus.Description2";
    const std::string args2        = "zeus.Args2";
    const std::string workingDir2  = zeus::CurrentExe::GetAppDir().u8string();
    const std::string icon2        = R"(C:\Program Files\Common Files\TortoiseOverlays\icons\Ribbon\AddedIcon.ico)";
    const int         showCmd      = SW_SHOWMAXIMIZED;
    {
        WinShellLink shellLink = WinShellLink::Create().value();

        shellLink.SetPath(targetPath.u8string());
        shellLink.SetIconLocation(icon1, 0);
        shellLink.SetWorkingDirectory(workingDir1);
        ASSERT_TRUE(shellLink.Save(linkPath.u8string()).has_value());
    }
    {
        auto shellLink = WinShellLink::Create();
        ASSERT_TRUE(shellLink->Load(linkPath.u8string()).has_value());
        EXPECT_EQ(targetPath.u8string(), shellLink->GetPath().value());
        EXPECT_EQ("", shellLink->GetArguments().value());
        EXPECT_EQ("", shellLink->GetDescription().value());
        EXPECT_EQ(icon1, shellLink->GetIconLocation()->first);
        EXPECT_EQ(0, shellLink->GetIconLocation()->second);
        EXPECT_EQ(workingDir1, shellLink->GetWorkingDirectory().value());
    }

    {
        WinShellLink shellLink = WinShellLink::Create().value();

        EXPECT_TRUE(shellLink.SetPath(targetPath.u8string()));
        EXPECT_TRUE(shellLink.SetArguments(args2));
        EXPECT_TRUE(shellLink.SetDescription(description2));
        EXPECT_TRUE(shellLink.SetIconLocation(icon2, 1));
        EXPECT_TRUE(shellLink.SetWorkingDirectory(workingDir2));
        EXPECT_TRUE(shellLink.SetShowCommand(showCmd));
        ASSERT_TRUE(shellLink.Save(linkPath.u8string()).has_value());
    }
    {
        auto shellLink = WinShellLink::Create();
        ASSERT_TRUE(shellLink->Load(linkPath.u8string()).has_value());
        EXPECT_EQ(targetPath.u8string(), shellLink->GetPath().value());
        EXPECT_EQ(args2, shellLink->GetArguments().value());
        EXPECT_EQ(description2, shellLink->GetDescription().value());
        EXPECT_EQ(icon2, shellLink->GetIconLocation()->first);
        EXPECT_EQ(1, shellLink->GetIconLocation()->second);
        EXPECT_EQ(workingDir2, shellLink->GetWorkingDirectory().value());
        EXPECT_EQ(showCmd, shellLink->GetShowCommand().value());
    }
}

TEST(WinShellLink, raw)
{
    fs::path        appDir     = zeus::CurrentExe::GetAppDir() / "symlinkTemp";
    fs::path        linkPath   = appDir / (zeus::Uuid::GenerateRandom().toString() + ".lnk");
    std::string     targetPath = u8R"(%ProgramFiles%/Windows Media Player/wmplayer.exe)";
    std::error_code code;
    fs::remove_all(appDir, code);
    fs::create_directories(appDir, code);
    zeus::AutoRelease autoClear {[&]()
                                 {
                                     fs::remove(linkPath, code);
                                     fs::remove(targetPath, code);
                                 }};
    auto              shellLink = WinShellLink::Create();
    shellLink->SetPath(targetPath);
    shellLink->Save(linkPath);
    EXPECT_EQ(targetPath, shellLink->GetRawPath().value());
}

TEST(WinFileDigtalSign, DigtalSign)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"谷歌@@应用.exe");

    auto file = WinFileDigtalSign::Load(filename, true);
    ASSERT_TRUE(file.has_value());
    ASSERT_EQ(1, file->GetDigtalSigns().size());
    const auto& sign = file->GetDigtalSigns().at(0);
    EXPECT_EQ("Google LLC", sign.GetSubjectName().value());
    EXPECT_EQ("0c15be4a15bb0903c901b1d6c265302f", sign.GetSerialNumber().value());
    EXPECT_EQ("DigiCert SHA2 Assured ID Code Signing CA", sign.GetIssuerName().value());
}

TEST(WinFileDigtalSign, NtDigtalSign)
{
    auto file = WinFileDigtalSign::Load(fs::u8path(R"(\??\)" + zeus::CurrentExe::GetAppDirString()) / fs::u8path(u8"谷歌@@应用.exe"), true);
    ASSERT_TRUE(file.has_value());
    ASSERT_EQ(1, file->GetDigtalSigns().size());
    const auto& sign = file->GetDigtalSigns().at(0);
    EXPECT_EQ("Google LLC", sign.GetSubjectName().value());
    EXPECT_EQ("0c15be4a15bb0903c901b1d6c265302f", sign.GetSerialNumber().value());
    EXPECT_EQ("DigiCert SHA2 Assured ID Code Signing CA", sign.GetIssuerName().value());
}

TEST(WinFileDigtalSign, MultiDigtalSign)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"RtlUpd.exe");

    auto file = WinFileDigtalSign::Load(filename, true);
    ASSERT_TRUE(file.has_value());
    ASSERT_EQ(2, file->GetDigtalSigns().size());
    const auto& sign1 = file->GetDigtalSigns().at(0);
    EXPECT_EQ("Realtek Semiconductor Corp", sign1.GetSubjectName().value());
    EXPECT_EQ("13222a5dccf716df5af9c87084412dd9", sign1.GetSerialNumber().value());
    EXPECT_EQ("VeriSign Class 3 Code Signing 2010 CA", sign1.GetIssuerName().value());
    const auto& sign2 = file->GetDigtalSigns().at(1);
    EXPECT_EQ("Microsoft Windows Hardware Compatibility Publisher", sign2.GetSubjectName().value());
    EXPECT_EQ("330000001dc31a761624754f8000000000001d", sign2.GetSerialNumber().value());
    EXPECT_EQ("Microsoft Windows Third Party Component CA 2012", sign2.GetIssuerName().value());
}

TEST(WinFileDigtalSign, MultiDigtalSign1)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"RtlUpd.exe");

    auto file = WinFileDigtalSign::Load(filename, false);
    ASSERT_TRUE(file.has_value());
    ASSERT_EQ(1, file->GetDigtalSigns().size());
    const auto& sign1 = file->GetDigtalSigns().at(0);
    EXPECT_EQ("Realtek Semiconductor Corp", sign1.GetSubjectName().value());
    EXPECT_EQ("13222a5dccf716df5af9c87084412dd9", sign1.GetSerialNumber().value());
    EXPECT_EQ("VeriSign Class 3 Code Signing 2010 CA", sign1.GetIssuerName().value());
}

TEST(WinFileManifest, manifest)
{
    fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"谷歌@@应用.exe");

    auto manifest = zeus::WinFileManifest::Load(filename);
    EXPECT_TRUE(manifest.has_value());
    EXPECT_EQ(zeus::WinFileManifest::Invoke::kRequireAdministrator, manifest->GetInvoke().value());
    EXPECT_TRUE(manifest->IsUiAccess().value());
}

TEST(WinFileManifest, Ntmanifest)
{
    auto manifest = zeus::WinFileManifest::Load(fs::u8path(R"(\??\)" + zeus::CurrentExe::GetAppDirString()) / fs::u8path(u8"谷歌@@应用.exe"));
    EXPECT_TRUE(manifest.has_value());
    EXPECT_EQ(zeus::WinFileManifest::Invoke::kRequireAdministrator, manifest->GetInvoke().value());
    EXPECT_TRUE(manifest->IsUiAccess().value());
}

TEST(WinFileIcon, GetRawFile)
{
    const fs::path appdir = zeus::CurrentExe::GetAppDir();

    auto tempdir = appdir / "iconTemp";
    {
        const fs::path  filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"max.exe");
        std::error_code ec;
        fs::remove_all(tempdir, ec);
        fs::create_directories(tempdir, ec);
        auto fileIcon = WinFileIconGroup::Open(filename);
        EXPECT_TRUE(fileIcon.has_value());
        auto tempname = (tempdir / "temp").replace_extension(".png");
        EXPECT_EQ(1, fileIcon->Icons().size());
        EXPECT_EQ(1, fileIcon->Icons().front().Images().size());
        EXPECT_EQ(WinFileIconImage::IconItemType::kPng, fileIcon->Icons().front().Images().front().Type());
        EXPECT_FALSE(fileIcon->Icons().front().Images().front().IsStandard());
        EXPECT_EQ("IDR_MAINFRAME", fileIcon->Icons().front().ResourceId());
        EXPECT_EQ(1, fileIcon->Icons().front().Images().front().Id());
        EXPECT_EQ(512, fileIcon->Icons().front().Images().front().Width());
        EXPECT_EQ(512, fileIcon->Icons().front().Images().front().Height());
        EXPECT_EQ(0, fileIcon->Icons().front().Images().front().EntryWidth());
        EXPECT_EQ(0, fileIcon->Icons().front().Images().front().EntryHeight());
        EXPECT_TRUE(fileIcon->Icons().front().Images().front().ExtractImage(tempname).has_value());
        EXPECT_TRUE(FileEqual(tempname, (appdir / "max.png")).value());
    }
    {
        fs::path        filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"谷歌@@应用.exe");
        std::error_code ec;
        fs::remove_all(tempdir, ec);
        fs::create_directories(tempdir, ec);
        auto fileIcon = WinFileIconGroup::Open(filename);
        EXPECT_TRUE(fileIcon.has_value());
        auto tempname = (tempdir / "temp").replace_extension(".png");
        EXPECT_EQ(10, fileIcon->Icons().size());
        EXPECT_EQ(7, fileIcon->Icons().at(9).Images().size());
        EXPECT_EQ(WinFileIconImage::IconItemType::kPng, fileIcon->Icons().at(9).Images().at(6).Type());
        EXPECT_TRUE(fileIcon->Icons().at(9).Images().at(6).IsStandard());
        EXPECT_EQ("IDR_X005_BETA", fileIcon->Icons().at(9).ResourceId());
        EXPECT_EQ(49, fileIcon->Icons().at(9).Images().at(6).Id());
        EXPECT_EQ(256, fileIcon->Icons().at(9).Images().at(6).Width());
        EXPECT_EQ(256, fileIcon->Icons().at(9).Images().at(6).Height());
        EXPECT_EQ(0, fileIcon->Icons().at(9).Images().at(6).EntryWidth());
        EXPECT_EQ(0, fileIcon->Icons().at(9).Images().at(6).EntryHeight());
        EXPECT_TRUE(fileIcon->Icons().at(9).Images().at(6).ExtractImage(tempname).has_value());
        EXPECT_TRUE(FileEqual(tempname, (appdir / "Icon49.png")).value());
    }
}

TEST(WinFileIcon, FileIconStand)
{
    const fs::path  appdir   = zeus::CurrentExe::GetAppDir();
    const fs::path  filename = appdir / fs::u8path(u8"max.exe");
    const auto      tempdir  = filename.parent_path() / "iconTemp";
    std::error_code ec;
    fs::remove_all(tempdir, ec);
    fs::create_directories(tempdir, ec);
    auto fileIcon = WinFileIconGroup::Open(filename);
    EXPECT_TRUE(fileIcon.has_value());
    EXPECT_EQ(1, fileIcon->Icons().size());
    auto tempname = (tempdir / "temp").replace_extension(".ico");
    EXPECT_TRUE(fileIcon->Icons().front().ExtractIconToFile(tempname, false).has_value());
    EXPECT_TRUE(FileEqual(tempname, (appdir / "max.ico")).value());
    EXPECT_FALSE(fileIcon->Icons().front().ExtractIconToFile(tempname, true).has_value());
}

TEST(WinFileIcon, NtFileIconPng)
{
    const fs::path filename = fs::u8path(R"(\??\)" + zeus::CurrentExe::GetAppDirString()) / fs::u8path(u8"max.exe");

    std::error_code ec;
    auto            fileIcon = WinFileIconGroup::Open(filename);
    EXPECT_TRUE(fileIcon.has_value());
    EXPECT_EQ(1, fileIcon->Icons().size());
    EXPECT_EQ(1, fileIcon->Icons().front().Images().size());
    EXPECT_EQ(WinFileIconImage::IconItemType::kPng, fileIcon->Icons().front().Images().front().Type());
    EXPECT_FALSE(fileIcon->Icons().front().Images().front().IsStandard());
    EXPECT_EQ("IDR_MAINFRAME", fileIcon->Icons().front().ResourceId());
    EXPECT_EQ(1, fileIcon->Icons().front().Images().front().Id());
    EXPECT_EQ(512, fileIcon->Icons().front().Images().front().Width());
    EXPECT_EQ(512, fileIcon->Icons().front().Images().front().Height());
    EXPECT_EQ(0, fileIcon->Icons().front().Images().front().EntryWidth());
    EXPECT_EQ(0, fileIcon->Icons().front().Images().front().EntryHeight());
}

TEST(WinFileIcon, FileIconSize)
{
    std::error_code ec;

    {
        const fs::path filename = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"谷歌@@应用.exe");
        auto           fileIcon = WinFileIconGroup::Open(filename);
        EXPECT_TRUE(fileIcon.has_value());
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(0);
            EXPECT_EQ(16, icon.Width());
            EXPECT_EQ(16, icon.Height());
            EXPECT_EQ(16, icon.EntryWidth());
            EXPECT_EQ(16, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kBmp, icon.Type());
            EXPECT_EQ(1, icon.Id());
            EXPECT_EQ(8, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(1);
            EXPECT_EQ(32, icon.Width());
            EXPECT_EQ(32, icon.Height());
            EXPECT_EQ(32, icon.EntryWidth());
            EXPECT_EQ(32, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kBmp, icon.Type());
            EXPECT_EQ(2, icon.Id());
            EXPECT_EQ(8, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(2);
            EXPECT_EQ(48, icon.Width());
            EXPECT_EQ(48, icon.Height());
            EXPECT_EQ(48, icon.EntryWidth());
            EXPECT_EQ(48, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kBmp, icon.Type());
            EXPECT_EQ(3, icon.Id());
            EXPECT_EQ(8, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(3);
            EXPECT_EQ(16, icon.Width());
            EXPECT_EQ(16, icon.Height());
            EXPECT_EQ(16, icon.EntryWidth());
            EXPECT_EQ(16, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kBmp, icon.Type());
            EXPECT_EQ(4, icon.Id());
            EXPECT_EQ(32, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(4);
            EXPECT_EQ(32, icon.Width());
            EXPECT_EQ(32, icon.Height());
            EXPECT_EQ(32, icon.EntryWidth());
            EXPECT_EQ(32, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kBmp, icon.Type());
            EXPECT_EQ(5, icon.Id());
            EXPECT_EQ(32, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(5);
            EXPECT_EQ(48, icon.Width());
            EXPECT_EQ(48, icon.Height());
            EXPECT_EQ(48, icon.EntryWidth());
            EXPECT_EQ(48, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kBmp, icon.Type());
            EXPECT_EQ(6, icon.Id());
            EXPECT_EQ(32, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
        {
            const auto& icon = fileIcon->Icons().at(0).Images().at(6);
            EXPECT_EQ(256, icon.Width());
            EXPECT_EQ(256, icon.Height());
            EXPECT_EQ(0, icon.EntryWidth());
            EXPECT_EQ(0, icon.EntryHeight());
            EXPECT_EQ(WinFileIconImage::IconItemType::kPng, icon.Type());
            EXPECT_EQ(7, icon.Id());
            EXPECT_EQ(32, icon.BitCount());
            EXPECT_TRUE(icon.IsStandard());
        }
    }
}

TEST(WinFileIcon, FileIconGet)
{
    constexpr int     kIconCounts       = 10;
    constexpr int     kIconItemCounts[] = {7, 1, 1, 1, 7, 7, 7, 4, 7, 7};
    const std::string kIconName[]       = {"IDR_MAINFRAME",     "IDR_MAINFRAME_2",       "IDR_MAINFRAME_3",    "IDR_MAINFRAME_4", "IDR_SXS",
                                           "IDR_X001_APP_LIST", "IDR_X002_APP_LIST_SXS", "IDR_X003_INCOGNITO", "IDR_X004_DEV",    "IDR_X005_BETA"};
    const fs::path    appdir            = zeus::CurrentExe::GetAppDir();
    const fs::path    filename          = zeus::CurrentExe::GetAppDir() / fs::u8path(u8"谷歌@@应用.exe");
    const auto        tempdir           = filename.parent_path() / "iconTemp";
    std::error_code   ec;
    fs::remove_all(tempdir, ec);
    fs::create_directories(tempdir, ec);
    auto fileIcon = WinFileIconGroup::Open(filename);
    EXPECT_TRUE(fileIcon.has_value());
    ASSERT_EQ(kIconCounts, fileIcon->Icons().size());
    auto tempname = (tempdir / "temp").replace_extension(".ico");
    for (size_t index = 0; index < fileIcon->Icons().size(); ++index)
    {
        const auto& icon = fileIcon->Icons().at(index);
        EXPECT_EQ(kIconItemCounts[index], icon.Images().size());
        EXPECT_EQ(kIconName[index], icon.ResourceId());
        EXPECT_TRUE(icon.ExtractIconToFile(tempname, false).has_value());
        EXPECT_TRUE(FileEqual(tempname, (appdir / kIconName[index]).replace_extension(".ico")).value());
    }
}

TEST(WinFileVersionInfo, base)
{
    auto file = WinFileVersionInfo::Load(zeus::CurrentExe::GetAppDir() / fs::u8path(u8"谷歌@@应用.exe"));
    ASSERT_TRUE(file.has_value());
    EXPECT_EQ(1, file->GetInfoList().size());
    EXPECT_EQ(file->GetInfoList().front().GetProductName().value(), "Google Chrome");
    EXPECT_EQ(file->GetFileVersion()->ToString(), "77.0.3865.120");
    EXPECT_EQ(file->GetProductVersion()->ToString(), "77.0.3865.120");
    EXPECT_EQ(file->GetInfoList().front().GetFileDescription().value(), "Google Chrome");
}

TEST(WinFileVersionInfo, Ntbase)
{
    auto file = WinFileVersionInfo::Load(fs::u8path(R"(\??\)" + zeus::CurrentExe::GetAppDirString()) / fs::u8path(u8"谷歌@@应用.exe"));
    ASSERT_TRUE(file.has_value());
    EXPECT_EQ(file->GetInfoList().front().GetProductName().value(), "Google Chrome");
    EXPECT_EQ(file->GetFileVersion()->ToString(), "77.0.3865.120");
    EXPECT_EQ(file->GetProductVersion()->ToString(), "77.0.3865.120");
    EXPECT_EQ(file->GetInfoList().front().GetFileDescription().value(), "Google Chrome");
}

TEST(WinFileAttributes, base)
{
    static const std::string kData   = "1234567890";
    const auto               testDir = fs::temp_directory_path() / "WinFileTest";
    std::error_code          ec;
    fs::remove_all(testDir, ec);
    fs::create_directories(testDir);
    auto filePath = testDir / "test.txt";
    auto now      = std::chrono::system_clock::now();
    {
        std::fstream(filePath, std::ios::out).close();
    }
    auto attributes = WinFileAttributes::Load(filePath);

    ASSERT_TRUE(attributes.has_value());
    EXPECT_EQ(attributes->GetSize(), 0);
    EXPECT_LT(attributes->GetCreationTime() - now, std::chrono::milliseconds(100));
    EXPECT_LT(attributes->GetLastAccessTime() - now, std::chrono::milliseconds(100));
    EXPECT_LT(attributes->GetLastWriteTime() - now, std::chrono::milliseconds(100));
    EXPECT_FALSE(attributes->IsNormal());
    EXPECT_TRUE(attributes->IsArchive());
    EXPECT_FALSE(attributes->IsCompress());
    EXPECT_FALSE(attributes->IsReadOnly());
    EXPECT_FALSE(attributes->IsHidden());
    EXPECT_TRUE(attributes->SetReadOnly(true));
    EXPECT_TRUE(attributes->IsReadOnly());
    {
        std::fstream file(filePath, std::ios::out);
        EXPECT_FALSE(file);
    }
    EXPECT_FALSE(attributes->IsNormal());
    EXPECT_TRUE(attributes->SetReadOnly(false));
    EXPECT_FALSE(attributes->IsReadOnly());
    {
        std::fstream file(filePath, std::ios::out);
        EXPECT_TRUE(file);
        file.write(kData.data(), kData.size());
    }
    EXPECT_TRUE(attributes->SetHidden(true));
    EXPECT_TRUE(attributes->IsHidden());
    EXPECT_FALSE(attributes->IsNormal());
    EXPECT_TRUE(attributes->SetHidden(false));
    EXPECT_FALSE(attributes->IsHidden());
    EXPECT_TRUE(attributes->SetArchive(false));
    EXPECT_FALSE(attributes->IsArchive());
    EXPECT_TRUE(attributes->IsNormal());
    EXPECT_TRUE(attributes->SetArchive(true));
    EXPECT_TRUE(attributes->IsArchive());
    EXPECT_EQ(attributes->GetSize(), kData.size());
    fs::remove_all(testDir, ec);
}

TEST(WinSystemService, base)
{
    auto service = WinSystemService::Open("Schedule", false, false);
    ASSERT_TRUE(service.has_value());
    EXPECT_EQ("Schedule", service->Name());
    EXPECT_NE(WinSystemService::ServiceType::kKernelDriver, service->GetServiceType());
    EXPECT_NE(WinSystemService::ServiceType::kFileSystemDriver, service->GetServiceType());
    EXPECT_EQ(WinSystemService::ServiceState::kRunning, service->GetServiceState().value());
    EXPECT_EQ(WinSystemService::StartupType::kAutoStart, service->GetStartupType().value());
    EXPECT_EQ("Task Scheduler", service->GetDisplayName().value());
    EXPECT_EQ(R"(C:\Windows\system32\svchost.exe -k netsvcs -p)", service->GetPath().value());
}

TEST(WinSystemService, save)
{
    if (zeus::WinToken::CurrentThreadHasAdmin().value())
    {
        std::string               serviceName        = "WinSystemServiceTestSave";
        std::string               serviceDisplayName = u8"WinSystemService测试";
        std::string               serviceDesc        = u8"WinSystemService测试描述";
        std::string               failCommand        = u8"failCommand";
        std::string               failMessage        = u8"failMessage";
        std::string               binPath            = R"(c:\Windows\System32)";
        std::vector<std::string>  depSet             = {"RpcSs", "nsi"};
        std::chrono::milliseconds failDelay(100);
        std::chrono::hours        resetPeriod(72);
        {
            auto service = WinSystemService::Open(serviceName, true, true);
            if (service.has_value())
            {
                ASSERT_TRUE(service->Remove().has_value());
            }
        }
        {
            auto service = WinSystemService::Create(
                serviceName, WinSystemService::ServiceType::kWin32OwnProcess, WinSystemService::StartupType::kManualStart, binPath, serviceDisplayName
            );
            ASSERT_TRUE(service.has_value());
            EXPECT_TRUE(service->SetDescription(serviceDesc).has_value());
            service->SetDependencies(depSet);
        }
        {
            auto service = WinSystemService::Open(serviceName, false, false);
            ASSERT_TRUE(service.has_value());
            EXPECT_EQ(serviceName, service->Name());
            EXPECT_EQ(WinSystemService::ServiceType::kWin32OwnProcess, service->GetServiceType());
            EXPECT_EQ(WinSystemService::StartupType::kManualStart, service->GetStartupType().value());
            EXPECT_FALSE(service->IsDelayAutoStart().value());
            EXPECT_EQ(serviceDisplayName, service->GetDisplayName().value());
            EXPECT_EQ(serviceDesc, service->GetDescription().value());
            EXPECT_EQ(binPath, service->GetPath());
            auto deps = service->GetDependencies().value();
            EXPECT_EQ(depSet, deps);
            const auto failAction = service->GetFailureAction();
            EXPECT_TRUE(failAction->GetActions().empty());
            EXPECT_EQ(0, failAction->GetResetPeriod()->count());
            EXPECT_TRUE(failAction->GetCommand().empty());
            EXPECT_TRUE(failAction->GetRebootMessage().empty());
        }
        {
            auto service = WinSystemService::Open(serviceName, true, true);
            ASSERT_TRUE(service.has_value());
            serviceDisplayName += "b";
            serviceDesc += "b";
            binPath += "b";
            EXPECT_TRUE(service->SetDisplayName(serviceDisplayName).has_value());
            EXPECT_TRUE(service->SetServiceType(WinSystemService::ServiceType::kWin32ShareProcess).has_value());
            EXPECT_TRUE(service->SetDescription(serviceDesc).has_value());
            EXPECT_TRUE(service->SetPath(binPath).has_value());
            EXPECT_TRUE(service->SetStartupType(WinSystemService::StartupType::kAutoStart).has_value());
            EXPECT_TRUE(service->SetDelayAutoStart(true));
            depSet.erase(depSet.begin());
            depSet.emplace_back("Schedule");
            EXPECT_TRUE(service->SetDependencies(depSet).has_value());
            WinSysterServiceFailureAction failAction;
            failAction.SetCommand(failCommand);
            failAction.SetRebootMessage(failMessage);
            failAction.SetResetPeriod(resetPeriod);
            failAction.AddAction(WinSysterServiceFailureAction::FailureActionType::kRestart, failDelay);
            failAction.AddAction(WinSysterServiceFailureAction::FailureActionType::kRestart, failDelay * 2);
            failAction.AddAction(WinSysterServiceFailureAction::FailureActionType::kRestart, failDelay * 3);
            EXPECT_TRUE(service->SetFailureAction(failAction).has_value());
        }
        {
            auto service = WinSystemService::Open(serviceName, false, false);
            ASSERT_TRUE(service.has_value());
            EXPECT_EQ(serviceName, service->Name());
            EXPECT_EQ(WinSystemService::ServiceType::kWin32ShareProcess, service->GetServiceType());
            EXPECT_EQ(WinSystemService::StartupType::kAutoStart, service->GetStartupType().value());
            EXPECT_TRUE(service->IsDelayAutoStart().value());
            EXPECT_EQ(serviceDisplayName, service->GetDisplayName().value());
            EXPECT_EQ(serviceDesc, service->GetDescription().value());
            EXPECT_EQ(binPath, service->GetPath());
            auto deps = service->GetDependencies().value();
            EXPECT_EQ(depSet, deps);
            const auto failAction = service->GetFailureAction();
            const auto actions    = failAction->GetActions();
            EXPECT_EQ(3, actions.size());
            EXPECT_EQ(WinSysterServiceFailureAction::FailureActionType::kRestart, actions.at(0).first);
            EXPECT_EQ(WinSysterServiceFailureAction::FailureActionType::kRestart, actions.at(1).first);
            EXPECT_EQ(WinSysterServiceFailureAction::FailureActionType::kRestart, actions.at(2).first);
            EXPECT_EQ(failDelay, actions.at(0).second);
            EXPECT_EQ(failDelay * 2, actions.at(1).second);
            EXPECT_EQ(failDelay * 3, actions.at(2).second);
            EXPECT_EQ(resetPeriod, failAction->GetResetPeriod().value());
            EXPECT_EQ(failCommand, failAction->GetCommand());
            EXPECT_EQ(failMessage, failAction->GetRebootMessage());
        }
        {
            auto service = WinSystemService::Open(serviceName, true, true);
            ASSERT_TRUE(service.has_value());
            EXPECT_TRUE(service->SetDependencies({}));
            auto failAction = service->GetFailureAction();
            failAction->ClearActions();
            failAction->SetCommand("");
            failAction->SetRebootMessage("");
            EXPECT_TRUE(service->SetFailureAction(failAction.value()).has_value());
        }
        {
            auto service = WinSystemService::Open(serviceName, false, false);
            ASSERT_TRUE(service.has_value());
            EXPECT_EQ(serviceName, service->Name());
            EXPECT_EQ(WinSystemService::ServiceType::kWin32ShareProcess, service->GetServiceType());
            EXPECT_EQ(WinSystemService::StartupType::kAutoStart, service->GetStartupType().value());
            EXPECT_EQ(serviceDisplayName, service->GetDisplayName().value());
            EXPECT_EQ(serviceDesc, service->GetDescription().value());
            EXPECT_EQ(binPath, service->GetPath());
            auto deps = service->GetDependencies().value();
            EXPECT_TRUE(deps.empty());
            const auto failAction = service->GetFailureAction();
            EXPECT_TRUE(failAction->GetActions().empty());
            EXPECT_EQ(0, failAction->GetResetPeriod()->count()); //删除actions后ResetPeriod自动设置为0
            EXPECT_TRUE(failAction->GetCommand().empty());
            EXPECT_TRUE(failAction->GetRebootMessage().empty());
        }
    }
}

TEST(WinSystemService, state)
{
    auto service = WinSystemService::Open("WpcMonSvc", false, false);
    ASSERT_TRUE(service.has_value());
    EXPECT_NE(WinSystemService::ServiceType::kKernelDriver, service->GetServiceType());
    EXPECT_NE(WinSystemService::ServiceType::kFileSystemDriver, service->GetServiceType());
    EXPECT_NE(WinSystemService::ServiceState::kRunning, service->GetServiceState().value());
}

TEST(WinSession, base)
{
    auto sessions = zeus::WinSession::ListAll();
    EXPECT_GE(sessions.size(), 2);
    bool session0 = false;
    for (auto const& session : sessions)
    {
        if (session.Id() == 0)
        {
            session0 = true;
            EXPECT_NE(WinSession::SessionState::Active, session.State());
        }
        if (session.State() == WinSession::SessionState::Active)
        {
            auto info = WinSession::GetSessionInfo(session.Id());
            EXPECT_FALSE(info->Username().empty());
            EXPECT_TRUE(info->LogonTime().value() >= info->ConnectTime().value());
        }
    }
    EXPECT_TRUE(session0);
}

TEST(SharedLibrary, base)
{
    auto library = SharedLibrary::Load(std::string("ntdll.dll"));
    EXPECT_TRUE(library.has_value());
    EXPECT_TRUE(library->HasSymbol("NtQuerySystemInformation"));
    auto module = LoadLibraryW(L"ntdll.dll");
    ASSERT_TRUE(module);
    EXPECT_EQ(library->GetSymbol("NtQuerySystemInformation"), GetProcAddress(module, "NtQuerySystemInformation"));
}

TEST(SharedLibrary, move)
{
    auto moveLibrary = SharedLibrary::Load(std::string("ntdll.dll"));
    EXPECT_TRUE(moveLibrary.has_value());
    EXPECT_TRUE(moveLibrary->HasSymbol("NtQuerySystemInformation"));
    auto library = SharedLibrary(std::move(*moveLibrary));
    EXPECT_TRUE(moveLibrary->Empty());
    EXPECT_FALSE(moveLibrary->GetSymbol("NtQuerySystemInformation"));
    EXPECT_TRUE(library.HasSymbol("NtQuerySystemInformation"));
    auto module = LoadLibraryW(L"ntdll.dll");
    ASSERT_TRUE(module);
    EXPECT_EQ(library.GetSymbol("NtQuerySystemInformation"), GetProcAddress(module, "NtQuerySystemInformation"));
}

TEST(SharedLibrary, Error)
{
    auto library = SharedLibrary::Load(std::string("1234567890.dll"));
    EXPECT_FALSE(library.has_value());
    std::string error = library.error().message();
    EXPECT_FALSE(error.empty());
    EXPECT_EQ(ERROR_MOD_NOT_FOUND, library.error().value());
}

TEST(WinFirewall, base)
{
    auto manager = WinFirewallPolicyManager::Create().value();

    const std::string kMdnsApp              = EnvironmentVariable::ExpandEnvironmentVariableString(R"(%SystemRoot%\system32\svchost.exe)");
    const std::string kMdnsInboundRuleName  = "mDNS (UDP-In)";
    const std::string kMdnsOutboundRuleName = "mDNS (UDP-Out)";
    const std::string kMdnsDesc             = "mDNS";
    {
        auto rule = manager.GetRule(kMdnsInboundRuleName);
        EXPECT_TRUE(rule.has_value());
        EXPECT_EQ(kMdnsInboundRuleName, rule->GetName().value());
        EXPECT_TRUE(StartWith(rule->GetDescription().value(), kMdnsDesc));
        EXPECT_EQ(WinFirewallRule::Protocol::kUdp, rule->GetProtocol().value());
        EXPECT_EQ("5353", rule->GetLocalPorts().value());
        EXPECT_EQ("*", rule->GetRemotePorts().value());
        EXPECT_EQ(WinFirewallRule::Direction::kInbound, rule->GetDirection().value());
        EXPECT_EQ(WinFirewallRule::Action::kAllow, rule->GetAction().value());
        EXPECT_EQ(kMdnsApp, rule->GetApplicationName().value());
        EXPECT_TRUE(rule->GetEnabled().value());
    }
    {
        auto rule = manager.GetRule(kMdnsOutboundRuleName);
        EXPECT_TRUE(rule.has_value());
        EXPECT_EQ(kMdnsOutboundRuleName, rule->GetName().value());
        EXPECT_TRUE(StartWith(rule->GetDescription().value(), kMdnsDesc));
        EXPECT_EQ(WinFirewallRule::Protocol::kUdp, rule->GetProtocol().value());
        EXPECT_EQ("*", rule->GetLocalPorts().value());
        EXPECT_EQ("5353", rule->GetRemotePorts().value());
        EXPECT_EQ(WinFirewallRule::Direction::kOutbound, rule->GetDirection().value());
        EXPECT_EQ(WinFirewallRule::Action::kAllow, rule->GetAction().value());
        EXPECT_EQ(kMdnsApp, rule->GetApplicationName().value());
        EXPECT_TRUE(rule->GetEnabled().value());
    }
    {
        auto rules = manager.ListAllRule();
        EXPECT_FALSE(rules->empty());
        bool findPrivateInbound  = false;
        bool findPrivateOutbound = false;
        bool findPublicInbound   = false;
        bool findPublicOutbound  = false;
        bool findDomainInbound   = false;
        bool findDomainOutbound  = false;
        for (auto& rule : *rules)
        {
            if (rule.GetName().value() == kMdnsInboundRuleName)
            {
                EXPECT_TRUE(StartWith(rule.GetDescription().value(), kMdnsDesc));
                EXPECT_EQ(WinFirewallRule::Protocol::kUdp, rule.GetProtocol().value());
                EXPECT_EQ("5353", rule.GetLocalPorts().value());
                EXPECT_EQ("*", rule.GetRemotePorts().value());
                EXPECT_EQ(WinFirewallRule::Direction::kInbound, rule.GetDirection().value());
                EXPECT_EQ(WinFirewallRule::Action::kAllow, rule.GetAction().value());
                EXPECT_EQ(kMdnsApp, rule.GetApplicationName().value());
                EXPECT_TRUE(rule.GetEnabled().value());
                switch (rule.GetProfile().value())
                {
                case WinFirewallRule::Profile::kPrivate:
                    findPrivateInbound = true;
                    break;
                case WinFirewallRule::Profile::kPublic:
                    findPublicInbound = true;
                    break;
                case WinFirewallRule::Profile::kDomain:
                    findDomainInbound = true;
                    break;
                default:
                    break;
                }
            }
            if (rule.GetName().value() == kMdnsOutboundRuleName)
            {
                EXPECT_TRUE(StartWith(rule.GetDescription().value(), kMdnsDesc));
                EXPECT_EQ(WinFirewallRule::Protocol::kUdp, rule.GetProtocol().value());
                EXPECT_EQ("*", rule.GetLocalPorts().value());
                EXPECT_EQ("5353", rule.GetRemotePorts().value());
                EXPECT_EQ(WinFirewallRule::Direction::kOutbound, rule.GetDirection().value());
                EXPECT_EQ(WinFirewallRule::Action::kAllow, rule.GetAction().value());
                EXPECT_EQ(kMdnsApp, rule.GetApplicationName().value());
                EXPECT_TRUE(rule.GetEnabled().value());
                switch (rule.GetProfile().value())
                {
                case WinFirewallRule::Profile::kPrivate:
                    findPrivateOutbound = true;
                    break;
                case WinFirewallRule::Profile::kPublic:
                    findPublicOutbound = true;
                    break;
                case WinFirewallRule::Profile::kDomain:
                    findDomainOutbound = true;
                    break;
                default:
                    break;
                }
            }
        }
        EXPECT_TRUE(findPrivateInbound);
        EXPECT_TRUE(findPrivateOutbound);
        EXPECT_TRUE(findPublicInbound);
        EXPECT_TRUE(findPublicOutbound);
        EXPECT_TRUE(findDomainInbound);
        EXPECT_TRUE(findDomainOutbound);
    }
    {
        ASSERT_TRUE(zeus::WinToken::CurrentProcessHasAdmin().value());
        auto name          = Uuid::GenerateRandom().toString();
        auto desc          = RandWord();
        auto localPort     = std::to_string(RandUint16());
        auto remotePort    = std::to_string(RandUint16());
        auto localAddress  = RandIpV4String();
        auto remoteAddress = RandIpV4String();
        auto app           = RandWord();
        auto group         = RandWord();
        {
            auto rule = manager.CreateRule();

            EXPECT_TRUE(rule->SetName(name).has_value());
            EXPECT_TRUE(rule->SetDescription(desc).has_value());
            EXPECT_TRUE(rule->SetEnabled(false).has_value());
            EXPECT_TRUE(rule->SetProfile(WinFirewallRule::Profile::kDomain | WinFirewallRule::Profile::kPrivate | WinFirewallRule::Profile::kPublic)
                            .has_value());
            EXPECT_TRUE(rule->SetDirection(WinFirewallRule::Direction::kInbound).has_value());
            EXPECT_TRUE(rule->SetAction(WinFirewallRule::Action::kBlock).has_value());
            EXPECT_TRUE(rule->SetProtocol(WinFirewallRule::Protocol::kTcp).has_value());
            EXPECT_TRUE(rule->SetLocalPorts(localPort).has_value());
            EXPECT_TRUE(rule->SetRemotePorts(remotePort).has_value());
            EXPECT_TRUE(rule->SetLocalAddresses(localAddress).has_value());
            EXPECT_TRUE(rule->SetRemoteAddresses(remoteAddress).has_value());
            EXPECT_TRUE(rule->SetApplicationName(app).has_value());
            EXPECT_TRUE(rule->SetGrouping(group).has_value());

            EXPECT_EQ(name, rule->GetName().value());
            EXPECT_EQ(desc, rule->GetDescription().value());
            EXPECT_FALSE(rule->GetEnabled().value());
            EXPECT_EQ(
                WinFirewallRule::Profile::kDomain | WinFirewallRule::Profile::kPrivate | WinFirewallRule::Profile::kPublic, rule->GetProfile().value()
            );
            EXPECT_EQ(WinFirewallRule::Direction::kInbound, rule->GetDirection().value());
            EXPECT_EQ(WinFirewallRule::Action::kBlock, rule->GetAction().value());
            EXPECT_EQ(WinFirewallRule::Protocol::kTcp, rule->GetProtocol().value());
            EXPECT_EQ(localPort, rule->GetLocalPorts().value());
            EXPECT_EQ(remotePort, rule->GetRemotePorts().value());
            EXPECT_TRUE(StartWith(rule->GetLocalAddresses().value(), localAddress));
            EXPECT_TRUE(StartWith(rule->GetRemoteAddresses().value(), remoteAddress));
            EXPECT_EQ(app, rule->GetApplicationName().value());
            EXPECT_EQ(group, rule->GetGrouping().value());

            EXPECT_TRUE(manager.AddRule(rule.value()).has_value());
        }
        {
            auto rule = manager.GetRule(name);
            EXPECT_TRUE(rule.has_value());
            EXPECT_EQ(name, rule->GetName().value());
            EXPECT_EQ(desc, rule->GetDescription().value());
            EXPECT_FALSE(rule->GetEnabled().value());
            EXPECT_EQ(
                WinFirewallRule::Profile::kDomain | WinFirewallRule::Profile::kPrivate | WinFirewallRule::Profile::kPublic, rule->GetProfile().value()
            );
            EXPECT_EQ(WinFirewallRule::Direction::kInbound, rule->GetDirection().value());
            EXPECT_EQ(WinFirewallRule::Action::kBlock, rule->GetAction().value());
            EXPECT_EQ(WinFirewallRule::Protocol::kTcp, rule->GetProtocol().value());
            EXPECT_EQ(localPort, rule->GetLocalPorts().value());
            EXPECT_EQ(remotePort, rule->GetRemotePorts().value());
            EXPECT_TRUE(StartWith(rule->GetLocalAddresses().value(), localAddress));
            EXPECT_TRUE(StartWith(rule->GetRemoteAddresses().value(), remoteAddress));
            EXPECT_EQ(app, rule->GetApplicationName().value());
            EXPECT_EQ(group, rule->GetGrouping().value());
        }
        {
            auto rule = manager.GetRule(name);
            EXPECT_TRUE(rule.has_value());
            EXPECT_TRUE(manager.RemoveRule(rule.value()).has_value());
        }
        {
            auto rule = manager.GetRule(name);
            EXPECT_FALSE(rule.has_value());
        }
    }
}
#endif
