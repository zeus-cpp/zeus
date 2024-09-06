#ifdef _WIN32
#include <vector>
#include <gtest/gtest.h>
#include <zeus/foundation/sync/event.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/string/charset_utils.h>
#include <zeus/foundation/system/win/registry.h>
#include <zeus/foundation/system/win/transaction.h>
#include <zeus/foundation/system/environment_variable.h>

using namespace zeus;

#define SUBKEY "TEST"

#include "wmi_registry.hpp"

TEST(WinRegistry, String)
{
    {
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
        ASSERT_TRUE(registry->SetStringValue(SUBKEY, SUBKEY).has_value());

        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        EXPECT_EQ(CharsetUtils::UnicodeToUTF8(aresWin.GetString(CharsetUtils::UTF8ToUnicode(SUBKEY))), SUBKEY);
    }
    {
        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        aresWin.SetString(CharsetUtils::UTF8ToUnicode(SUBKEY), L"AbcXyz");

        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);

        EXPECT_EQ(registry->GetStringValue(SUBKEY).value(), "AbcXyz");
    }
    {
        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());
        std::string test;
        for (auto i = 0; i < 100; i++)
        {
            test.append("asdaasadadWWWDDDaWdsad");
        }

        aresWin.SetString(CharsetUtils::UTF8ToUnicode(SUBKEY), CharsetUtils::UTF8ToUnicode(test));

        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);

        EXPECT_EQ(registry->GetStringValue(SUBKEY).value(), test);
    }
}

TEST(WinRegistry, StringExpand)
{
    {
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
        ASSERT_TRUE(registry->SetStringExpandValue(SUBKEY, R"(%programdata%)"));

        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        EXPECT_EQ(CharsetUtils::UnicodeToUTF8(aresWin.GetStringExpand(CharsetUtils::UTF8ToUnicode(SUBKEY))), EnvironmentVariable::Get("programdata"));
    }
    {
        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        aresWin.SetStringExpand(CharsetUtils::UTF8ToUnicode(SUBKEY), CharsetUtils::UTF8ToUnicode(R"(%appdata%)"));

        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);

        EXPECT_EQ(registry->GetStringExpandValue(SUBKEY).value(), EnvironmentVariable::Get("appdata"));
    }
}

TEST(WinRegistry, Binary)
{
    std::vector<char> bin {'1', '0', '1'};
    {
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
        registry->SetBinaryValue(SUBKEY, bin.data(), bin.size());

        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        auto result = aresWin.GetBinary(CharsetUtils::UTF8ToUnicode(SUBKEY));

        EXPECT_EQ(bin.size(), result.size());
        EXPECT_TRUE(std::equal(bin.begin(), bin.end(), result.begin()));
    }
    {
        bin = {'1', '1', '1', '1'};
        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        aresWin.SetBinary(CharsetUtils::UTF8ToUnicode(SUBKEY), bin);

        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);

        auto result = registry->GetBinaryValue(SUBKEY).value();

        EXPECT_EQ(bin.size(), result.size());
        EXPECT_TRUE(std::equal(bin.begin(), bin.end(), result.begin()));
    }
}

TEST(WinRegistry, Int)
{
    {
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
        ASSERT_TRUE(registry->SetDwordValue(SUBKEY, (std::numeric_limits<int>::max)()).has_value());

        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        EXPECT_EQ(aresWin.GetInt(CharsetUtils::UTF8ToUnicode(SUBKEY)), (std::numeric_limits<int>::max)());
    }
    {
        Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

        ASSERT_TRUE(aresWin.Init());

        aresWin.SetInt(CharsetUtils::UTF8ToUnicode(SUBKEY), (std::numeric_limits<int>::min)());

        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);

        EXPECT_EQ(registry->GetDwordValue(SUBKEY).value(), (std::numeric_limits<int>::min)());
    }
}

TEST(WinRegistry, Int64)
{
    auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
    ASSERT_TRUE(registry->SetQwordValue(SUBKEY, (std::numeric_limits<long long>::max)()).has_value());

    Ares::WinRegistry aresWin(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(SUBKEY));

    ASSERT_TRUE(aresWin.Init());

    EXPECT_EQ(aresWin.GetInt64(CharsetUtils::UTF8ToUnicode(SUBKEY)), (std::numeric_limits<long long>::max)());
}

TEST(WinRegistry, ExistsValue)
{
    {
        WinRegistry::DeleteKey(HKEY_CURRENT_USER, SUBKEY);
        EXPECT_FALSE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
        EXPECT_FALSE(registry->ExistsValue(SUBKEY).value());
        EXPECT_TRUE(registry->SetDwordValue(SUBKEY, 1).has_value());
        EXPECT_TRUE(registry->ExistsValue(SUBKEY).value());
    }
    {
        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);
        EXPECT_TRUE(registry->ExistsValue(SUBKEY).value());
        EXPECT_EQ(1, registry->GetDwordValue(SUBKEY).value());
    }
}

TEST(WinRegistry, ExistsKey)
{
    {
        WinRegistry::DeleteKey(HKEY_CURRENT_USER, SUBKEY);
        EXPECT_FALSE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);
        EXPECT_TRUE(registry->SetDwordValue(SUBKEY, 1).has_value());
        EXPECT_TRUE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
    }
    {
        EXPECT_TRUE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
        auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);
        EXPECT_EQ(1, registry->GetDwordValue(SUBKEY).value());
    }
}

TEST(WinRegistry, SubKeys)
{
    WinRegistry::DeleteKey(HKEY_CURRENT_USER, SUBKEY);
    auto registry1 = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY "\\asd", true, true);
    EXPECT_TRUE(registry1->SetDwordValue("1", 0).has_value());

    auto registry2 = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY "\\fgh", true, true);
    EXPECT_TRUE(registry2->SetDwordValue("1", 0).has_value());
    auto                     registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);
    std::vector<std::string> keys     = *registry->GetSubKeys();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_EQ(keys[0], "asd");
    EXPECT_EQ(keys[1], "fgh");
}

TEST(WinRegistry, DeleteExists)
{
    {
        auto registry = WinRegistry::CreateKey(HKEY_CURRENT_USER, SUBKEY, true, true);

        registry->SetDwordValue(SUBKEY, 1);

        EXPECT_TRUE(registry->ExistsValue(SUBKEY).value());

        EXPECT_TRUE(registry->DeleteValue(SUBKEY).has_value());

        EXPECT_FALSE(registry->ExistsValue(SUBKEY).value());
    }
    EXPECT_TRUE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
    EXPECT_TRUE(WinRegistry::DeleteKey(HKEY_CURRENT_USER, SUBKEY).has_value());
    EXPECT_FALSE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
}

TEST(WinRegistry, View)
{
    {
        auto registryWin32 =
            WinRegistry::CreateKey(R"(HKEY_CURRENT_USER\Software\Classes\CLSID\TEST)", true, true, WinRegistry::ViewType::WIN_KEY_32);
        auto registryWin64 =
            WinRegistry::CreateKey(R"(HKEY_CURRENT_USER\Software\Classes\CLSID\TEST)", true, true, WinRegistry::ViewType::WIN_KEY_64);

        ASSERT_TRUE(registryWin32->SetDwordValue("bit", 32).has_value());
        ASSERT_TRUE(registryWin64->SetDwordValue("bit", 64).has_value());

        Ares::WinRegistry aresWin32(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(R"(Software\Classes\CLSID\TEST)"), 32);
        Ares::WinRegistry aresWin64(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(R"(Software\Classes\CLSID\TEST)"), 64);

        ASSERT_TRUE(aresWin32.Init());
        ASSERT_TRUE(aresWin64.Init());

        EXPECT_EQ(aresWin32.GetInt(L"bit"), 32);
        EXPECT_EQ(aresWin64.GetInt(L"bit"), 64);
    }
    {
        Ares::WinRegistry aresWin32(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(R"(Software\Classes\CLSID\TEST)"), 32);
        Ares::WinRegistry aresWin64(HKEY_CURRENT_USER, CharsetUtils::UTF8ToUnicode(R"(Software\Classes\CLSID\TEST)"), 64);

        ASSERT_TRUE(aresWin32.Init());
        ASSERT_TRUE(aresWin64.Init());

        aresWin32.SetInt(L"bit", 320);
        aresWin64.SetInt(L"bit", 640);

        auto registryWin32 = WinRegistry::OpenKey(R"(HKEY_CURRENT_USER\Software\Classes\CLSID\TEST)", true, false, WinRegistry::ViewType::WIN_KEY_32);
        auto registryWin64 = WinRegistry::OpenKey(R"(HKEY_CURRENT_USER\Software\Classes\CLSID\TEST)", true, false, WinRegistry::ViewType::WIN_KEY_64);

        EXPECT_EQ(registryWin32->GetDwordValue("bit").value(), 320);
        EXPECT_EQ(registryWin64->GetDwordValue("bit").value(), 640);
    }
}

TEST(WinRegistry, Transaction)
{
    auto stringValue = RandWord();
    auto key1        = RandWord();
    auto key2        = RandWord();
    WinRegistry::DeleteKey(HKEY_CURRENT_USER, SUBKEY);

    auto transaction = WinTransaction::Create("zeus-Test");
    ASSERT_TRUE(transaction.has_value());
    {
        auto transactionRegistry = WinRegistry::CreateTransactedKey(transaction->Handle(), HKEY_CURRENT_USER, SUBKEY, true, true);
        ASSERT_TRUE(transactionRegistry.has_value());
        ASSERT_TRUE(transactionRegistry->SetDwordValue(key1, 1).has_value());
        EXPECT_FALSE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
        ASSERT_TRUE(transactionRegistry->SetStringValue(key2, stringValue).has_value());
        EXPECT_FALSE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
        transactionRegistry->Close();
    }
    EXPECT_FALSE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
    transaction->Commit();
    EXPECT_TRUE(WinRegistry::ExistsKey(HKEY_CURRENT_USER, SUBKEY).value());
    auto registry = WinRegistry::OpenKey(HKEY_CURRENT_USER, SUBKEY, true, false);
    EXPECT_EQ(registry->GetDwordValue(key1).value(), 1);
    EXPECT_EQ(registry->GetStringValue(key2).value(), stringValue);
    WinRegistry::DeleteKey(HKEY_CURRENT_USER, SUBKEY);
}

#endif
