#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#include <gtest/gtest.h>
#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/string/charset_utils.h>

using namespace zeus;

TEST(Environment, GetHome)
{
    wchar_t path[MAX_PATH] = {0};
    SHGetSpecialFolderPathW(NULL, path, CSIDL_PROFILE, FALSE);
    EXPECT_EQ(zeus::CharsetUtils::UnicodeToUTF8(path), Environment::GetHomeString());
}

#endif