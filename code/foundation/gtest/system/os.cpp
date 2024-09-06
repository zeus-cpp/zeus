#include <gtest/gtest.h>
#include <zeus/foundation/system/os.h>

TEST(OS, ProductType)
{
    zeus::OS::OsProductType();
}

TEST(OS, OsArchitecture)
{
    EXPECT_FALSE(zeus::OS::OsArchitecture().empty());
}