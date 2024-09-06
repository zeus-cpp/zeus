#include <gtest/gtest.h>
#include <random>
#include <bitset>
#include <chrono>
#include <limits>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <zeus/foundation/core/system_error.h>
#include <zeus/foundation/core/win/com_error.h>
#include <zeus/foundation/core/singleton.hpp>

using namespace std;
using namespace std::chrono;
using namespace zeus;

#ifdef _WIN32
TEST(Error, Com)
{
    std::error_code err(HRESULT(E_OUTOFMEMORY), WinComCategory::ErrorCategory());
    EXPECT_EQ(E_OUTOFMEMORY, err.value());
    auto msg = err.message();
    EXPECT_FALSE(msg.empty());
}
#endif

TEST(Error, SystemError)
{
    std::error_code err(2, SystemCategory());
    EXPECT_EQ(2, err.value());
    auto msg = err.message();
    EXPECT_FALSE(msg.empty());

    {
        SystemError     value {2};
        std::error_code ec1(value);
        std::error_code ec2(2, SystemCategory());
        EXPECT_EQ(ec1, ec2);
        EXPECT_EQ(ec1.category(), ec2.category());
    }
}

TEST(Singleton, Instance)
{
    class Single
    {
    };

    Single* a = &Singleton<Single>::Instance();
    Single* b = &Singleton<Single>::Instance();
    EXPECT_EQ(a, b);
}

TEST(RefCountSingleton, Instance)
{
    static int global = 6;
    global            = 6;
    class Single
    {
    public:
        Single() { *_data = 5; }
        ~Single() { *_data = 4; }
        int  Data() { return *_data; }
        int* _data = &global;
    };
    EXPECT_EQ(global, 6);
    RefCountSingleton<Single>::IncRef();
    EXPECT_EQ(global, 5);
    Single* a = &RefCountSingleton<Single>::Instance();
    Single* b = &RefCountSingleton<Single>::Instance();
    EXPECT_EQ(a, b);
    EXPECT_EQ(RefCountSingleton<Single>::Instance().Data(), 5);
    RefCountSingleton<Single>::DecRef();
    EXPECT_EQ(global, 4);
}
