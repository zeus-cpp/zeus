#ifdef _WIN32
#include <gtest/gtest.h>
#include <zeus/foundation/security/win/token.h>
#include <zeus/foundation/security/win/privilege.h>

using namespace zeus;

TEST(WinToken, base)
{
    auto token     = WinToken::GetCurrentProcessToken();
    auto duplicate = token.Duplicate(true, WinToken::SecurityImpersonationLevel::Impersonation);
    ASSERT_TRUE(duplicate.has_value());
    ASSERT_TRUE(duplicate.value() != nullptr);
    auto sid = duplicate->GetUserSID();
    ASSERT_TRUE(sid.has_value());
    ASSERT_TRUE(sid.value() != nullptr);
    auto sidString = duplicate->GetUserSIDString();
    ASSERT_TRUE(sidString.has_value());
    EXPECT_EQ(*WinSID::GetSIDString(*sid), *sidString);
    auto account = duplicate->GetUserAccount();
    ASSERT_TRUE(account.has_value());
    EXPECT_EQ(WinSID::GetSIDAccount(*sid)->Username(), account->Username());
    EXPECT_EQ(WinSID::GetSIDAccount(*sid)->Domain(), account->Domain());
    EXPECT_EQ(*WinSID::GetSIDAccount(*sid), *account);
    Process::GetCurrentProcess().SessionId(), duplicate->GetSessionId();
    EXPECT_TRUE(*(duplicate->IsPrimary()));
    EXPECT_TRUE(*(token.IsPrimary()));
    auto data = duplicate->GetInformation(TokenGroups);
    ASSERT_TRUE(data.has_value());
    ASSERT_FALSE(data->empty());
    TOKEN_GROUPS* groups = reinterpret_cast<TOKEN_GROUPS*>(data->data());
    EXPECT_GT(groups->GroupCount, 0);
    ASSERT_TRUE(token.EnablePrivileges(WinPrivilege::Privilege::kSE_SHUTDOWN_NAME, true).has_value());
    EXPECT_TRUE(token.HasPrivilege(WinPrivilege::Privilege::kSE_SHUTDOWN_NAME).value());
}

TEST(WinToken, Process)
{
    auto token = WinToken::GetProcessToken(Process::GetCurrentId());
    ASSERT_TRUE(token.has_value());
    ASSERT_TRUE(token.has_value());
    ASSERT_TRUE(token.value() != nullptr);
    auto sid = token->GetUserSID();
    ASSERT_TRUE(sid.has_value());
    ASSERT_TRUE(sid.value() != nullptr);
    auto sidString = token->GetUserSIDString();
    ASSERT_TRUE(sidString.has_value());
    EXPECT_EQ(*WinSID::GetSIDString(*sid), *sidString);
    auto account = token->GetUserAccount();
    ASSERT_TRUE(account.has_value());
    EXPECT_EQ(WinSID::GetSIDAccount(*sid)->Username(), account->Username());
    EXPECT_EQ(WinSID::GetSIDAccount(*sid)->Domain(), account->Domain());
    EXPECT_EQ(*WinSID::GetSIDAccount(*sid), *account);
    Process::GetCurrentProcess().SessionId(), token->GetSessionId();
    EXPECT_TRUE(*(token->IsPrimary()));
    auto data = token->GetInformation(TokenGroups);
    ASSERT_TRUE(data.has_value());
    ASSERT_FALSE(data->empty());
    TOKEN_GROUPS* groups = reinterpret_cast<TOKEN_GROUPS*>(data->data());
    EXPECT_GT(groups->GroupCount, 0);
}

TEST(WinPrivilege, base)
{
    auto privilege = WinPrivilege::GetPrivilege(WinPrivilege::Privilege::kSE_SHUTDOWN_NAME);
    ASSERT_TRUE(privilege.has_value());
    auto name = *(privilege->GetName());
    EXPECT_EQ(WinPrivilege::CastPrivilegeName(WinPrivilege::Privilege::kSE_SHUTDOWN_NAME), name);
}

#endif