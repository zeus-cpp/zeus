#include <gtest/gtest.h>
#include <zeus/foundation/compiler/compiler.h>

using namespace zeus;
TEST(Compiler, ClassName)
{
    class A
    {
    };
    struct B
    {
    };
    EXPECT_EQ("A", GetClassName(typeid(A)));
    EXPECT_EQ("B", GetClassName(typeid(B)));
    EXPECT_EQ("int", GetClassName(typeid(int)));
    EXPECT_EQ("void", GetClassName(typeid(void)));
}