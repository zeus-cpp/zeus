#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/string/charset_utils.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/string/url_utils.h>
#include <zeus/foundation/string/version.h>
#include "charsetutils_string.h"

#include <codecvt>
#ifdef WIN32
#include <Windows.h>
#endif // WIN32

using namespace zeus;
using namespace std;

TEST(CharsetUtils, UnicodeToUTF8)
{
    std::string expString     = CHARSETUTILS_UTF8_STRING;
    std::string expLongString = CHARSETUTILS_UTF8_LONG_STRING;

    EXPECT_EQ(CharsetUtils::UnicodeToUTF8(CHARSETUTILS_UNICODE_STRING), expString);
    EXPECT_EQ(CharsetUtils::UnicodeToUTF8(CHARSETUTILS_UNICODE_LONG_STRING), expLongString);
}

TEST(CharsetUtils, UTF8ToUnicode)
{
    std::wstring expString     = CHARSETUTILS_UNICODE_STRING;
    std::wstring expLongString = CHARSETUTILS_UNICODE_LONG_STRING;

    EXPECT_EQ(CharsetUtils::UTF8ToUnicode(CHARSETUTILS_UTF8_STRING), expString);
    EXPECT_EQ(CharsetUtils::UTF8ToUnicode(CHARSETUTILS_UTF8_LONG_STRING), expLongString);
}

#ifdef WIN32
TEST(CharsetUtils, ANSI)
{
    EXPECT_EQ(CharsetUtils::ANSIToUnicode(CharsetUtils::UnicodeToANSI(CHARSETUTILS_UNICODE_STRING)), CHARSETUTILS_UNICODE_STRING);
    EXPECT_EQ(CharsetUtils::ANSIToUnicode(CharsetUtils::UnicodeToANSI(CHARSETUTILS_UNICODE_LONG_STRING)), CHARSETUTILS_UNICODE_LONG_STRING);
    EXPECT_EQ(CharsetUtils::ANSIToUTF8(CharsetUtils::UTF8ToANSI(CHARSETUTILS_UTF8_STRING)), CHARSETUTILS_UTF8_STRING);
    EXPECT_EQ(CharsetUtils::ANSIToUTF8(CharsetUtils::UTF8ToANSI(CHARSETUTILS_UTF8_LONG_STRING)), CHARSETUTILS_UTF8_LONG_STRING);
}

#endif // WIN32

TEST(CharsetUtils, UTF8Count)
{
    EXPECT_EQ(7, *CharsetUtils::UTF8CharPrintCount(u8"阿斯蒂芬刚刚好"));
    EXPECT_EQ(7, *CharsetUtils::UTF8CharPrintCount(u8"asdfghj"));
    EXPECT_EQ(7, *CharsetUtils::UTF8CharPrintCount("asdfghj"));
    EXPECT_EQ(5, *CharsetUtils::UTF8CharPrintCount(std::string(5, 110)));
}

TEST(StringOperation, Icompare)
{
    const string a = "aaa";
    const string b = "aaa";
    const string c = "aaaa";
    const string d = "aAa";

    EXPECT_EQ(0, zeus::Icompare(a, b));
    EXPECT_EQ(-1, zeus::Icompare(a, c));
    EXPECT_EQ(0, zeus::Icompare(a, d));

    const wstring wa = L"你大爷";
    const wstring wb = L"你大爷";
    const wstring wc = L"我大爷";

    EXPECT_EQ(0, zeus::Icompare(wa, wb));
    EXPECT_NE(0, zeus::Icompare(wa, wc));
}

TEST(StringOperation, IEqual)
{
    const string a = "aaa";
    const string b = "aaa";
    const string c = "aaaa";
    const string d = "aAa";

    EXPECT_TRUE(zeus::IEqual(a, b));
    EXPECT_TRUE(zeus::IEqual(a, d));
    EXPECT_TRUE(zeus::IEqual(d, a));
    EXPECT_FALSE(zeus::IEqual(a, c));
    EXPECT_FALSE(zeus::IEqual(c, d));
    EXPECT_TRUE(zeus::IEqual("123d4sd56fa789", "123d4sd56fa789"));
    EXPECT_TRUE(zeus::IEqual("asdfgh", "asdfgH"));
    EXPECT_FALSE(zeus::IEqual("asdfgh", "asdfgHi"));

    const wstring wa = L"你大爷";
    const wstring wb = L"你大爷";
    const wstring wc = L"我大爷";

    EXPECT_TRUE(zeus::IEqual(wa, wb));
    EXPECT_FALSE(zeus::IEqual(wa, wc));
}

TEST(StringOperation, IsNumber)
{
    const string a = "546464646";
    const string b = "a1215s";

    EXPECT_TRUE(zeus::IsNumber(a));
    EXPECT_FALSE(zeus::IsNumber(b));

    EXPECT_TRUE(zeus::IsNumber(L"832094703958452"));
    EXPECT_FALSE(zeus::IsNumber(L"9234732702j893890432"));
}

TEST(StringOperation, SplitString)
{
    string strA = "++A+B++C++";
    auto   ret  = zeus::Split(strA, "+");

    vector<string> ans {"A", "B", "C"};

    bool res = ans.size() == ret.size();
    for (unsigned int i = 0; i < ret.size(); ++i)
    {
        if (ret.at(i) != ans.at(i))
        {
            res = false;
            break;
        }
    }

    EXPECT_TRUE(res);
}

TEST(StringOperation, SplitString1)
{
    string strA = "A+B++C";
    auto   ret  = zeus::Split(strA, "+");

    vector<string> ans {"A", "B", "C"};

    bool res = ans.size() == ret.size();
    for (unsigned int i = 0; i < ret.size(); ++i)
    {
        if (ret.at(i) != ans.at(i))
        {
            res = false;
            break;
        }
    }

    EXPECT_TRUE(res);
}

TEST(StringOperation, SplitWString)
{
    wstring strB = L"++你大爷+我大爷++C++";
    auto    ret  = zeus::Split(strB, L"+");

    vector<wstring> ans {L"你大爷", L"我大爷", L"C"};

    bool res = ans.size() == ret.size();
    for (unsigned int i = 0; i < ret.size(); ++i)
    {
        if (ret.at(i) != ans.at(i))
        {
            res = false;
            break;
        }
    }

    EXPECT_TRUE(res);
}

TEST(StringOperation, SplitMultiWString)
{
    wchar_t               multiData1[] = L"++你大爷\0+我大爷\0++C++\0\0\0\0";
    wchar_t               multiData2[] = L"++你大爷\0+我大爷\0++C++\0\0";
    const vector<wstring> expect {L"++你大爷", L"+我大爷", L"++C++"};
    {
        auto result = SplitMultiWString(multiData1, sizeof(multiData1));
        EXPECT_EQ(expect, result);
    }
    {
        auto result = SplitMultiWString(multiData2, sizeof(multiData2));
        EXPECT_EQ(expect, result);
    }
}

TEST(StringOperation, SplitMultiString)
{
    char                 multiData1[] = "++你大爷\0+我大爷\0++C++\0\0\0\0";
    char                 multiData2[] = "++你大爷\0+我大爷\0++C++\0\0";
    const vector<string> expect {"++你大爷", "+我大爷", "++C++"};
    {
        auto result = SplitMultiString(multiData1, sizeof(multiData1));
        EXPECT_EQ(expect, result);
    }
    {
        auto result = SplitMultiString(multiData2, sizeof(multiData2));
        EXPECT_EQ(expect, result);
    }
}

TEST(StringOperation, JoinString)
{
    vector<string> data {"AB", "BC", "CD"};
    EXPECT_EQ(zeus::Join(data, "+"), "AB+BC+CD");
    EXPECT_EQ(zeus::Join(data, ""), "ABBCCD");
    EXPECT_EQ(zeus::Join({}, "#"), "");
    EXPECT_EQ(zeus::Join({"AB"}, "#"), "AB");
}

TEST(StringOperation, JoinWString)
{
    vector<wstring> data {L"AB", L"BC", L"CD"};
    EXPECT_EQ(zeus::Join(data, L"+"), L"AB+BC+CD");
    EXPECT_EQ(zeus::Join(data, L""), L"ABBCCD");
    EXPECT_EQ(zeus::Join({}, L"#"), L"");
    EXPECT_EQ(zeus::Join({L"AB"}, L"#"), L"AB");
}

TEST(StringOperation, Trim)
{
    EXPECT_EQ(std::string("bb"), zeus::Trim(" \n bb  \t "));
    EXPECT_EQ(std::string("aa"), zeus::Trim("  aa   "));
    EXPECT_EQ(std::string(""), zeus::Trim(""));

    EXPECT_EQ(std::string("bb  \t "), zeus::TrimBegin(" \n bb  \t "));
    EXPECT_EQ(std::string(" \n bb"), zeus::TrimEnd(" \n bb  \t "));
    EXPECT_EQ(std::string(""), zeus::TrimBegin(""));
    EXPECT_EQ(std::string(""), zeus::TrimEnd(""));

    EXPECT_EQ(std::wstring(L"bb"), zeus::Trim(L" \n bb  \t "));
    EXPECT_EQ(std::wstring(L"aa"), zeus::Trim(L"  aa   "));
    EXPECT_EQ(std::wstring(L""), zeus::Trim(L""));

    EXPECT_EQ(std::wstring(L"bb  \t "), zeus::TrimBegin(L" \n bb  \t "));
    EXPECT_EQ(std::wstring(L" \n bb"), zeus::TrimEnd(L" \n bb  \t "));
    EXPECT_EQ(std::wstring(L""), zeus::TrimBegin(L""));
    EXPECT_EQ(std::wstring(L""), zeus::TrimEnd(L""));
}

TEST(StringOperation, Repeat)
{
    std::string str("AbcXyzaa");
    EXPECT_EQ(zeus::Repeat(str, 5), "AbcXyzaaAbcXyzaaAbcXyzaaAbcXyzaaAbcXyzaa");
    std::wstring wstr(L"AbcXyz我");
    EXPECT_EQ(zeus::Repeat(wstr, 5), L"AbcXyz我AbcXyz我AbcXyz我AbcXyz我AbcXyz我");
}

TEST(StringOperation, EndWith)
{
    std::string str("AbcXyzaa");
    EXPECT_TRUE(zeus::EndWith(str, ""));
    EXPECT_TRUE(zeus::EndWith("", ""));
    EXPECT_FALSE(zeus::EndWith("", "ff"));
    EXPECT_TRUE(zeus::EndWith(str, "aa"));
    EXPECT_FALSE(zeus::EndWith(str, "bb"));
    EXPECT_FALSE(zeus::EndWith(str, "bbAbcXyzaab"));
    std::wstring wstr(L"AbcXyz我");
    EXPECT_TRUE(zeus::EndWith(wstr, L"我"));
    EXPECT_FALSE(zeus::EndWith(wstr, L"你"));
    EXPECT_FALSE(zeus::EndWith(wstr, L"我AbcXyz我"));
    std::string str1("AbcXyza");
    EXPECT_FALSE(zeus::EndWith(str1, "engssHipaa"));
}

TEST(StringOperation, StartWith)
{
    std::string str("aaAbcXyz");
    EXPECT_TRUE(zeus::StartWith(str, ""));
    EXPECT_TRUE(zeus::StartWith("", ""));
    EXPECT_FALSE(zeus::StartWith("", "ff"));
    EXPECT_TRUE(zeus::StartWith(str, "aa"));
    EXPECT_FALSE(zeus::StartWith(str, "bb"));
    EXPECT_FALSE(zeus::StartWith(str, "bbaaAbcXyz"));
    std::wstring wstr(L"我AbcXyz");
    EXPECT_TRUE(zeus::StartWith(wstr, L"我"));
    EXPECT_FALSE(zeus::StartWith(wstr, L"你"));
    EXPECT_FALSE(zeus::StartWith(wstr, L"你你我AbcXyz"));
}
TEST(StringOperation, Replace)
{
    std::string str("aaAbcXyz");
    EXPECT_EQ(std::string("bbbbAbcXyz"), zeus::Replace(str, "aa", "bbbb"));
    std::wstring wstr(L"我是AbcXyz");
    EXPECT_EQ(std::wstring(L"你AbcXyz"), zeus::Replace(wstr, L"我是", L"你"));
}

TEST(StringOperation, Replace1)
{
    std::string str("aaAbcXyz");
    EXPECT_EQ(std::string("aaAbcXyz"), zeus::Replace(str, "aadasdasdasdsdadadasdadsdasdad", "bbbb"));
    std::wstring wstr(L"aaAbcXyz");
    EXPECT_EQ(std::wstring(L"aaAbcXyz"), zeus::Replace(wstr, L"aadasdasdasdsdadadasdadsdasdad", L"bbbb"));
}

TEST(StringOperation, Replace2)
{
    std::string str("aaAbcXyz");
    EXPECT_EQ(std::string("AbcXyz"), zeus::Replace(str, "aa", ""));
    std::wstring wstr(L"AbcXyz");
    EXPECT_EQ(std::wstring(L"AbcXyz"), zeus::Replace(wstr, L"aa", L""));
}

TEST(Version, Parse)
{
    const auto fun = [](const std::string& delims, const std::string& prefix)
    {
        const auto major   = RandUint32();
        const auto minor   = RandUint32();
        const auto patch   = RandUint32();
        const auto build   = RandUint32();
        auto       version = Version::Parse(
            prefix + std::to_string(major) + delims + std::to_string(minor) + delims + std::to_string(patch) + delims + std::to_string(build), true
        );
        if (!prefix.empty())
        {
            EXPECT_FALSE(version.has_value());
            version = Version::Parse(
                prefix + std::to_string(major) + delims + std::to_string(minor) + delims + std::to_string(patch) + delims + std::to_string(build),
                false
            );
            ASSERT_TRUE(version.has_value());
        }
        else
        {
            ASSERT_TRUE(version.has_value());
        }
        EXPECT_FALSE(version->Zero());
        EXPECT_EQ(version->GetMajor(), major);
        EXPECT_EQ(version->GetMinor(), minor);
        EXPECT_EQ(version->GetPatch(), patch);
        EXPECT_EQ(version->GetBuild(), build);

        version = Version::Parse(prefix + std::to_string(major) + delims + std::to_string(minor) + delims + std::to_string(patch), true);
        if (!prefix.empty())
        {
            EXPECT_FALSE(version.has_value());
            version = Version::Parse(prefix + std::to_string(major) + delims + std::to_string(minor) + delims + std::to_string(patch), false);
            ASSERT_TRUE(version.has_value());
        }
        else
        {
            ASSERT_TRUE(version.has_value());
        }
        EXPECT_FALSE(version->Zero());
        EXPECT_EQ(version->GetMajor(), major);
        EXPECT_EQ(version->GetMinor(), minor);
        EXPECT_EQ(version->GetPatch(), patch);
        EXPECT_EQ(version->GetBuild(), 0);

        version = Version::Parse(prefix + std::to_string(major) + delims + std::to_string(minor), true);
        if (!prefix.empty())
        {
            EXPECT_FALSE(version.has_value());
            version = Version::Parse(prefix + std::to_string(major) + delims + std::to_string(minor), false);
            ASSERT_TRUE(version.has_value());
        }
        else
        {
            ASSERT_TRUE(version.has_value());
        }
        EXPECT_FALSE(version->Zero());
        EXPECT_EQ(version->GetMajor(), major);
        EXPECT_EQ(version->GetMinor(), minor);
        EXPECT_EQ(version->GetPatch(), 0);
        EXPECT_EQ(version->GetBuild(), 0);

        version = Version::Parse(prefix + std::to_string(major), true);
        if (!prefix.empty())
        {
            EXPECT_FALSE(version.has_value());
            version = Version::Parse(prefix + std::to_string(major), false);
            ASSERT_TRUE(version.has_value());
        }
        else
        {
            ASSERT_TRUE(version.has_value());
        }
        EXPECT_FALSE(version->Zero());
        EXPECT_EQ(version->GetMajor(), major);
        EXPECT_EQ(version->GetMinor(), 0);
        EXPECT_EQ(version->GetPatch(), 0);
        EXPECT_EQ(version->GetBuild(), 0);
    };
    fun(".", "");
    fun("-", "");
    fun(".", "v");
    fun("-", "v");
}

TEST(Version, Strange)
{
    {
        auto version = Version::Parse("...", false);
        EXPECT_FALSE(version.has_value());
    }
    {
        auto version = Version::Parse("...", true);
        EXPECT_FALSE(version.has_value());
    }
}

TEST(Version, Compare)
{
    Version v1(100, 223, 5678, 33);
    Version v2(100, 223, 5677, 33);
    Version v3(100, 223, 5677, 34);
    Version v4(101, 223, 5677, 33);
    Version v5(101, 223, 5677, 33);
    EXPECT_LT(v2, v1);
    EXPECT_GT(v1, v2);

    EXPECT_GT(v4, v1);
    EXPECT_LT(v1, v4);

    EXPECT_GT(v4, v3);
    EXPECT_LT(v3, v4);

    EXPECT_EQ(v4, v5);
    EXPECT_LE(v4, v5);
    EXPECT_GE(v4, v5);
}

TEST(Version, Set)
{
    auto major   = RandUint32();
    auto minor   = RandUint32();
    auto patch   = RandUint32();
    auto build   = RandUint32();
    auto version = Version::Parse(std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) + "." + std::to_string(build));
    EXPECT_FALSE(version->Zero());
    EXPECT_EQ(version->GetMajor(), major);
    EXPECT_EQ(version->GetMinor(), minor);
    EXPECT_EQ(version->GetPatch(), patch);
    EXPECT_EQ(version->GetBuild(), build);
    major = RandUint32();
    minor = RandUint32();
    patch = RandUint32();
    build = RandUint32();
    version->SetMajor(major);
    version->SetMinor(minor);
    version->SetPatch(patch);
    version->SetBuild(build);
    EXPECT_EQ(version->GetMajor(), major);
    EXPECT_EQ(version->GetMinor(), minor);
    EXPECT_EQ(version->GetPatch(), patch);
    EXPECT_EQ(version->GetBuild(), build);
}

TEST(ToHex, Int)
{
    EXPECT_EQ("00000D45", zeus::IntToHexString(3397));
    EXPECT_EQ("00000D45", zeus::IntToHexString(3397U));
    EXPECT_EQ("00000d45", zeus::IntToHexString(3397, false));
    EXPECT_EQ("00000d45", zeus::IntToHexString(3397U, false));
    EXPECT_EQ("0000FA8888888888", zeus::IntToHexString((int64_t) 0xFA8888888888));
    EXPECT_EQ("0000FA8888888888", zeus::IntToHexString((uint64_t) 0xFA8888888888));
    EXPECT_EQ("0000fa8888888888", zeus::IntToHexString((int64_t) 0xFA8888888888, false));
    EXPECT_EQ("0000fa8888888888", zeus::IntToHexString((uint64_t) 0xFA8888888888, false));
}

TEST(Url, base)
{
    const std::map<std::string, std::string> params = {
        {   "name",                           "John Doe"},
        {    "age",                                 "30"},
        {   "city",                           "New York"},
        {"country",                                "USA"},
        {  "query",                      "Hello, World!"},
        {"special",                        "!@#$%^&*(+)"},
        {"unicode",                       "你好世界"},
        {  "email",                   "user@example.com"},
        {    "url", "https://example.com/path?key=value"}
    };
    const std::string url =
        R"(age=30&city=New%20York&country=USA&email=user%40example.com&name=John%20Doe&query=Hello%2C%20World%21&special=%21%40%23%24%25%5E%26%2A%28%2B%29&unicode=%E4%BD%A0%E5%A5%BD%E4%B8%96%E7%95%8C&url=https%3A%2F%2Fexample.com%2Fpath%3Fkey%3Dvalue)";
    EXPECT_EQ(url, GenerateUrlQueryString(params));
    for (const auto& [key, value] : params)
    {
        EXPECT_EQ(UriUnescape(UriEscape(key)), key);
        EXPECT_EQ(UriUnescape(UriEscape(value)), value);
    }
}
