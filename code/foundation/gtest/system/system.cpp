#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/system/environment_variable.h>

using namespace zeus;

TEST(EnvironmentVariable, base)
{
    auto key   = zeus::RandWord();
    auto value = zeus::RandWord();
    EXPECT_FALSE(zeus::EnvironmentVariable::Has(key));
    EXPECT_FALSE(zeus::EnvironmentVariable::Get(key).has_value());
    EXPECT_TRUE(zeus::EnvironmentVariable::Set(key, value));
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    EXPECT_EQ(value, zeus::EnvironmentVariable::Get(key).value());
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    EXPECT_EQ(value, zeus::EnvironmentVariable::Get(key));
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    value = zeus::RandWord();
    EXPECT_TRUE(zeus::EnvironmentVariable::Set(key, value));
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    EXPECT_EQ(value, zeus::EnvironmentVariable::Get(key).value());
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    EXPECT_EQ(value, zeus::EnvironmentVariable::Get(key).value());
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    zeus::EnvironmentVariable::Unset(key);
    EXPECT_FALSE(zeus::EnvironmentVariable::Has(key));
    EXPECT_FALSE(zeus::EnvironmentVariable::Get(key).has_value());
    value = zeus::RandWord();
    EXPECT_TRUE(zeus::EnvironmentVariable::Set(key, value));
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    EXPECT_EQ(value, zeus::EnvironmentVariable::Get(key).value());
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
    EXPECT_EQ(value, zeus::EnvironmentVariable::Get(key).value());
    EXPECT_TRUE(zeus::EnvironmentVariable::Has(key));
}

TEST(EnvironmentVariable, Parse)
{
    {
        char data[] = "key1=va;lue1\0key2=val=ue2\0key3=value3\0\0\0\0";
        auto result = zeus::EnvironmentVariable::ParseEnvironmentVariableData(data, sizeof(data));
        EXPECT_EQ(3, result.size());
        EXPECT_EQ("va;lue1", result["key1"]);
        EXPECT_EQ("val=ue2", result["key2"]);
        EXPECT_EQ("value3", result["key3"]);
    }
    {
        wchar_t data[] = L"key1=va;lue1\0key2=val=ue2\0key3=value3\0\0\0\0";
        auto    result = zeus::EnvironmentVariable::ParseEnvironmentVariableDataW(data, sizeof(data));
        EXPECT_EQ(3, result.size());
        EXPECT_EQ(L"va;lue1", result[L"key1"]);
        EXPECT_EQ(L"val=ue2", result[L"key2"]);
        EXPECT_EQ(L"value3", result[L"key3"]);
    }
    {
        std::map<std::string, std::string> data = {
            {"key1", "va;lue1"},
            {"key2", "val=ue2"},
            {"key3",  "value3"},
        };
        auto result = zeus::EnvironmentVariable::CreateEnvironmentVariableData(data);
        EXPECT_EQ(39, result.size());
        auto parse = zeus::EnvironmentVariable::ParseEnvironmentVariableData(result.data());
        EXPECT_EQ(data, parse);
    }
    {
        std::map<std::wstring, std::wstring> wdata = {
            {L"key1", L"va;lue1"},
            {L"key2", L"val=ue2"},
            {L"key3",  L"value3"},
        };
        auto result = zeus::EnvironmentVariable::CreateEnvironmentVariableData(wdata);
        EXPECT_EQ(39 * sizeof(wchar_t), result.size());
        auto parse = zeus::EnvironmentVariable::ParseEnvironmentVariableDataW(result.data());
        EXPECT_EQ(wdata, parse);
    }
}

TEST(Process, Cmdline)
{
    for (const auto& item : Process::ListAll())
    {
        auto cmd = Process::GetProcessCmdlineArguments(item.Id());
    }
}

TEST(Process, SpecialCmdline)
{
    {
        std::string cmdline =
            u8R"("C:\Program   Files\randomapp2387-desktop\randomapp2387汉字exe.exe"   --type=renderer   --user-data-dir="C:\Users  \bjr\AppData\Roa\"mi'ngrandomapp2387' 'des' ktop"   --standard-schemes=randomapp2387-browser --secure-schemes=randomapp2387-browser   --bypasscsp-schemes=randomapp2387-browser --cors-schemes --fetch-schemes=randomapp2387-browser --service-worker-schemes=randomapp2387-browser   --streaming-schemes   --app-path="C:\Progra  m Fil es\randomapp2387-desktop\resources\app.asar"   --no-sandbox --no-zygote --js-flags=--expose-gc --lang=zh-CN --device-scale-factor=1 --num-raster-threads=4 --enable-main-frame-before-activation --renderer-client-id=4 --launch-time-ticks=33416179 --mojo-platform-channel-handle=3108 --field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072 --disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand /prefetch:1     )";
        std::vector<std::string> expect = {
            u8R"(C:\Program   Files\randomapp2387-desktop\randomapp2387汉字exe.exe)",
            u8R"(--type=renderer)",
            u8R"(--user-data-dir="C:\Users  \bjr\AppData\Roa\"mi'ngrandomapp2387' 'des' ktop")",
            u8R"(--standard-schemes=randomapp2387-browser)",
            u8R"(--secure-schemes=randomapp2387-browser)",
            u8R"(--bypasscsp-schemes=randomapp2387-browser)",
            u8R"(--cors-schemes)",
            u8R"(--fetch-schemes=randomapp2387-browser)",
            u8R"(--service-worker-schemes=randomapp2387-browser)",
            u8R"(--streaming-schemes)",
            u8R"(--app-path="C:\Progra  m Fil es\randomapp2387-desktop\resources\app.asar")",
            u8R"(--no-sandbox)",
            u8R"(--no-zygote)",
            u8R"(--js-flags=--expose-gc)",
            u8R"(--lang=zh-CN)",
            u8R"(--device-scale-factor=1)",
            u8R"(--num-raster-threads=4)",
            u8R"(--enable-main-frame-before-activation)",
            u8R"(--renderer-client-id=4)",
            u8R"(--launch-time-ticks=33416179)",
            u8R"(--mojo-platform-channel-handle=3108)",
            u8R"(--field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072)",
            u8R"(--disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand)",
            u8R"(/prefetch:1)",
        };
        auto result = Process::SplitCmdline(cmdline);
        EXPECT_EQ(expect, *result);
    }

    {
        std::string cmdline =
            u8R"('C:\Program   Files\randomapp2387-desktop\randomapp2387汉字exe.exe'   --type=renderer   --user-data-dir="C:\Users  \bjr\AppData\Roa\"mi'ngrandomapp2387' 'des' ktop"   --standard-schemes=randomapp2387-browser --secure-schemes=randomapp2387-browser   --bypasscsp-schemes=randomapp2387-browser --cors-schemes --fetch-schemes=randomapp2387-browser --service-worker-schemes=randomapp2387-browser   --streaming-schemes   --app-path='C:\Progra  m Fil es\randomapp2387-desktop\resources\app.asar'   --no-sandbox --no-zygote --js-flags=--expose-gc --lang=zh-CN --device-scale-factor=1 --num-raster-threads=4 --enable-main-frame-before-activation --renderer-client-id=4 --launch-time-ticks=33416179 --mojo-platform-channel-handle=3108 --field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072 --disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand /prefetch:1     )";
        std::vector<std::string> expect = {
            u8R"(C:\Program   Files\randomapp2387-desktop\randomapp2387汉字exe.exe)",
            u8R"(--type=renderer)",
            u8R"(--user-data-dir="C:\Users  \bjr\AppData\Roa\"mi'ngrandomapp2387' 'des' ktop")",
            u8R"(--standard-schemes=randomapp2387-browser)",
            u8R"(--secure-schemes=randomapp2387-browser)",
            u8R"(--bypasscsp-schemes=randomapp2387-browser)",
            u8R"(--cors-schemes)",
            u8R"(--fetch-schemes=randomapp2387-browser)",
            u8R"(--service-worker-schemes=randomapp2387-browser)",
            u8R"(--streaming-schemes)",
            u8R"(--app-path='C:\Progra  m Fil es\randomapp2387-desktop\resources\app.asar')",
            u8R"(--no-sandbox)",
            u8R"(--no-zygote)",
            u8R"(--js-flags=--expose-gc)",
            u8R"(--lang=zh-CN)",
            u8R"(--device-scale-factor=1)",
            u8R"(--num-raster-threads=4)",
            u8R"(--enable-main-frame-before-activation)",
            u8R"(--renderer-client-id=4)",
            u8R"(--launch-time-ticks=33416179)",
            u8R"(--mojo-platform-channel-handle=3108)",
            u8R"(--field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072)",
            u8R"(--disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand)",
            u8R"(/prefetch:1)",
        };
        auto result = Process::SplitCmdline(cmdline);
        EXPECT_EQ(expect, *result);
    }

    {
        std::string cmdline =
            u8R"("C:\Program   Files\randomapp2387-desktop\randomapp2387汉字exe.exe"   --type=renderer'   --user-data-dir="C:\Users  \bjr\AppData\Roa\"mi'ngrandomapp2387' 'des' ktop"   --standard-schemes=randomapp2387-browser --secure-schemes=randomapp2387-browser   --bypasscsp-schemes=randomapp2387-browser --cors-schemes --fetch-schemes=randomapp2387-browser --service-worker-schemes=randomapp2387-browser   --streaming-schemes   --app-path="C:\Progra  m Fil es\randomapp2387-desktop\resources\app.asar"   --no-sandbox --no-zygote --js-flags=--expose-gc --lang=zh-CN --device-scale-factor=1 --num-raster-threads=4 --enable-main-frame-before-activation --renderer-client-id=4 --launch-time-ticks=33416179 --mojo-platform-channel-handle=3108 --field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072 --disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand /prefetch:1     )";
        std::vector<std::string> expect = {
            u8R"(C:\Program   Files\randomapp2387-desktop\randomapp2387汉字exe.exe)",
            u8R"(--type=renderer)",
            u8R"(--user-data-dir="C:\Users  \bjr\AppData\Roa\"mi'ngrandomapp2387' 'des' ktop")",
            u8R"(--standard-schemes=randomapp2387-browser)",
            u8R"(--secure-schemes=randomapp2387-browser)",
            u8R"(--bypasscsp-schemes=randomapp2387-browser)",
            u8R"(--cors-schemes)",
            u8R"(--fetch-schemes=randomapp2387-browser)",
            u8R"(--service-worker-schemes=randomapp2387-browser)",
            u8R"(--streaming-schemes)",
            u8R"(--app-path="C:\Progra  m Fil es\randomapp2387-desktop\resources\app.asar")",
            u8R"(--no-sandbox)",
            u8R"(--no-zygote)",
            u8R"(--js-flags=--expose-gc)",
            u8R"(--lang=zh-CN)",
            u8R"(--device-scale-factor=1)",
            u8R"(--num-raster-threads=4)",
            u8R"(--enable-main-frame-before-activation)",
            u8R"(--renderer-client-id=4)",
            u8R"(--launch-time-ticks=33416179)",
            u8R"(--mojo-platform-channel-handle=3108)",
            u8R"(--field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072)",
            u8R"(--disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand)",
            u8R"(/prefetch:1)",
        };
        auto result = Process::SplitCmdline(cmdline);
        EXPECT_FALSE(result);
    }

    {
        std::string cmdline =
            u8R"(C:\Program-Files\randomapp2387-desktop\randomapp2387汉字exe.exe --type=renderer --user-data-dir="C:\Users\bjr\AppData\Roaming\randomapp2387-desktop" --standard-schemes=randomapp2387-browser --secure-schemes=randomapp2387-browser --bypasscsp-schemes=randomapp2387-browser --cors-schemes --fetch-schemes=randomapp2387-browser --service-worker-schemes=randomapp2387-browser --streaming-schemes --app-path="C:\Program Files\randomapp2387-desktop\resources\app.asar" --no-sandbox --no-zygote --js-flags=--expose-gc --lang=zh-CN --device-scale-factor=1 --num-raster-threads=4 --enable-main-frame-before-activation --renderer-client-id=4 --launch-time-ticks=33416179 --mojo-platform-channel-handle=3108 --field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072 --disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand /prefetch:1)";
        std::vector<std::string> expect = {
            u8R"(C:\Program-Files\randomapp2387-desktop\randomapp2387汉字exe.exe)",
            u8R"(--type=renderer)",
            u8R"(--user-data-dir="C:\Users\bjr\AppData\Roaming\randomapp2387-desktop")",
            u8R"(--standard-schemes=randomapp2387-browser)",
            u8R"(--secure-schemes=randomapp2387-browser)",
            u8R"(--bypasscsp-schemes=randomapp2387-browser)",
            u8R"(--cors-schemes)",
            u8R"(--fetch-schemes=randomapp2387-browser)",
            u8R"(--service-worker-schemes=randomapp2387-browser)",
            u8R"(--streaming-schemes)",
            u8R"(--app-path="C:\Program Files\randomapp2387-desktop\resources\app.asar")",
            u8R"(--no-sandbox)",
            u8R"(--no-zygote)",
            u8R"(--js-flags=--expose-gc)",
            u8R"(--lang=zh-CN)",
            u8R"(--device-scale-factor=1)",
            u8R"(--num-raster-threads=4)",
            u8R"(--enable-main-frame-before-activation)",
            u8R"(--renderer-client-id=4)",
            u8R"(--launch-time-ticks=33416179)",
            u8R"(--mojo-platform-channel-handle=3108)",
            u8R"(--field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072)",
            u8R"(--disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand)",
            u8R"(/prefetch:1)",
        };
        auto result = Process::SplitCmdline(cmdline);
        EXPECT_EQ(expect, *result);
    }

    {
        std::string cmdline =
            u8R"("C:\Program-Files\randomapp2387-desktop\randomapp2387汉字exe.exe --type=renderer --user-data-dir="C:\Users\bjr\AppData\Roaming\randomapp2387-desktop" --standard-schemes=randomapp2387-browser --secure-schemes=randomapp2387-browser --bypasscsp-schemes=randomapp2387-browser --cors-schemes --fetch-schemes=randomapp2387-browser --service-worker-schemes=randomapp2387-browser --streaming-schemes --app-path="C:\Program Files\randomapp2387-desktop\resources\app.asar" --no-sandbox --no-zygote --js-flags=--expose-gc --lang=zh-CN --device-scale-factor=1 --num-raster-threads=4 --enable-main-frame-before-activation --renderer-client-id=4 --launch-time-ticks=33416179 --mojo-platform-channel-handle=3108 --field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072 --disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand /prefetch:1)";
        std::vector<std::string> expect = {
            u8R"(C:\Program-Files\randomapp2387-desktop\randomapp2387汉字exe.exe)",
            u8R"(--type=renderer)",
            u8R"(--user-data-dir="C:\Users\bjr\AppData\Roaming\randomapp2387-desktop")",
            u8R"(--standard-schemes=randomapp2387-browser)",
            u8R"(--secure-schemes=randomapp2387-browser)",
            u8R"(--bypasscsp-schemes=randomapp2387-browser)",
            u8R"(--cors-schemes)",
            u8R"(--fetch-schemes=randomapp2387-browser)",
            u8R"(--service-worker-schemes=randomapp2387-browser)",
            u8R"(--streaming-schemes)",
            u8R"(--app-path="C:\Program Files\randomapp2387-desktop\resources\app.asar")",
            u8R"(--no-sandbox)",
            u8R"(--no-zygote)",
            u8R"(--js-flags=--expose-gc)",
            u8R"(--lang=zh-CN)",
            u8R"(--device-scale-factor=1)",
            u8R"(--num-raster-threads=4)",
            u8R"(--enable-main-frame-before-activation)",
            u8R"(--renderer-client-id=4)",
            u8R"(--launch-time-ticks=33416179)",
            u8R"(--mojo-platform-channel-handle=3108)",
            u8R"(--field-trial-handle=2576,i,16251200598100236357,8248315928713620812,131072)",
            u8R"(--disable-features=SpareRendererForSitePerProcess,WinRetrieveSuggestionsOnlyOnDemand)",
            u8R"(/prefetch:1)",
        };
        auto result = Process::SplitCmdline(cmdline);
        EXPECT_FALSE(result);
    }

    {
        std::string              cmdline = u8R"("C:\Windows\system32\cmd.exe"    /c     "pause /4")";
        std::vector<std::string> expect  = {
            u8R"(C:\Windows\system32\cmd.exe)",
            u8R"(/c)",
            u8R"(pause /4)",
        };
        auto result = Process::SplitCmdline(cmdline);
        EXPECT_EQ(expect, *result);
    }
}
