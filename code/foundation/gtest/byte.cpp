#include <array>
#include <cstring>
#include <gtest/gtest.h>
#include <zeus/foundation/byte/byte_order.h>
#include <zeus/foundation/byte/byte_utils.h>
#include <zeus/foundation/string/charset_utils.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/core/random.h>

using namespace zeus;

TEST(Byte, Find)
{
    uint8_t data[] = {'t', 'e', 's', 't', 0, 1, 1, 2, 3, 4};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}), 0);
}

TEST(Byte, Find1)
{
    uint8_t data[] = {1, 1, 2, 0, 't', 'e', 's', 't', 0, 1, 2, 3, 4};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}), 4);
}

TEST(Byte, Find2)
{
    uint8_t data[] = {1, 0, 2, 3, 4, 't', 'e', 's', 't'};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}), 5);
}

TEST(Byte, Find3)
{
    uint8_t data[] = {
        1, 0, 2, 3, 4, 54, 4, 3, 45, 3,
    };
    uint8_t sub[] = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}), std::string::npos);
}

TEST(Byte, Find4)
{
    uint8_t data[] = {1, 0, 2, 3, 4, 't', 'e', 's', 't'};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}, 3), 5);
}

TEST(Byte, StrideFind1)
{
    uint8_t data[] = {1, 0, 2, 3, 4, 'e', 'e', 'e', 't'};
    uint8_t sub[]  = {
        'e',
        'e',
    };
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}, 0, 2), 6);
}

TEST(Byte, StrideFind2)
{
    uint8_t data[] = {1, 0, 2, 3, 4, 'e', 'e', 'r', 't'};
    uint8_t sub[]  = {
        'e',
        'e',
    };
    EXPECT_EQ(ByteFind({data, sizeof(data)}, {sub, sizeof(sub)}, 0, 2), std::string::npos);
}

TEST(Byte, RFind)
{
    uint8_t data[] = {'t', 'e', 's', 't', 0, 1, 1, 2, 3, 4};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteReverseFind({data, sizeof(data)}, {sub, sizeof(sub)}), 0);
}

TEST(Byte, RFind1)
{
    uint8_t data[] = {1, 1, 2, 0, 't', 'e', 's', 't', 0, 1, 2, 3, 4};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteReverseFind({data, sizeof(data)}, {sub, sizeof(sub)}), 4);
}

TEST(Byte, RFind2)
{
    uint8_t data[] = {1, 0, 2, 3, 4, 't', 'e', 's', 't'};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteReverseFind({data, sizeof(data)}, {sub, sizeof(sub)}), 5);
}

TEST(Byte, RFind3)
{
    uint8_t data[] = {
        1, 0, 2, 3, 4, 54, 4, 3, 45, 3,
    };
    uint8_t sub[] = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteReverseFind({data, sizeof(data)}, {sub, sizeof(sub)}), std::string::npos);
}

TEST(Byte, RFind4)
{
    uint8_t data[] = {1, 1, 2, 0, 't', 'e', 's', 't', 0, 1, 2, 3, 4, 7, 8};
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    EXPECT_EQ(ByteReverseFind({data, sizeof(data)}, {sub, sizeof(sub)}, 2), 4);
}

TEST(Byte, Split)
{
    uint8_t data1[] = {0, 1, 2, 3, 4};
    uint8_t data2[] = {0, 1, 2, 3, 4, 5};
    uint8_t data3[] = {0, 1, 2, 3, 4, 5, 6};
    uint8_t data4[] = {0, 1, 2, 3, 4, 7, 8};
    uint8_t data5[] = {0, 1, 2, 3, 4, 89, 0, 9, 9};
    uint8_t sub[]   = {'t', 'e', 's', 't'};
    uint8_t data[sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(data5) + sizeof(sub) * 4];
    std::memcpy(data, data1, sizeof(data1));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 0, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 1, data2, sizeof(data2));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 1, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 2, data3, sizeof(data3));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(sub) * 2, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(sub) * 3, data4, sizeof(data4));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(sub) * 3, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(sub) * 4, data5, sizeof(data5));

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 5);
    EXPECT_EQ(0, std::memcmp(data1, ret[0].Data(), sizeof(data1)));
    EXPECT_EQ(sizeof(data1), ret[0].Size());
    EXPECT_EQ(0, std::memcmp(data2, ret[1].Data(), sizeof(data2)));
    EXPECT_EQ(sizeof(data2), ret[1].Size());
    EXPECT_EQ(0, std::memcmp(data3, ret[2].Data(), sizeof(data3)));
    EXPECT_EQ(sizeof(data3), ret[2].Size());
    EXPECT_EQ(0, std::memcmp(data4, ret[3].Data(), sizeof(data4)));
    EXPECT_EQ(sizeof(data4), ret[3].Size());
    EXPECT_EQ(0, std::memcmp(data5, ret[4].Data(), sizeof(data5)));
    EXPECT_EQ(sizeof(data5), ret[4].Size());
}

TEST(Byte, Split1)
{
    uint8_t data1[] = {0, 1, 2, 3, 4};
    uint8_t data2[] = {0, 1, 2, 3, 4, 5};
    uint8_t data3[] = {0, 1, 2, 3, 4, 5, 6};
    uint8_t data4[] = {0, 1, 2, 3, 4, 7, 8};
    uint8_t data5[] = {0, 1, 2, 3, 4, 89, 0, 9, 9};
    uint8_t sub[]   = {'t', 'e', 's', 't'};
    uint8_t data[sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(data5) + sizeof(sub) * 5];
    std::memcpy(data + sizeof(sub) * 0, sub, sizeof(sub));
    std::memcpy(data + sizeof(sub) * 1, data1, sizeof(data1));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 1, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 2, data2, sizeof(data2));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 2, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 3, data3, sizeof(data3));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(sub) * 3, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(sub) * 4, data4, sizeof(data4));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(sub) * 4, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(sub) * 5, data5, sizeof(data5));

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 5);
    EXPECT_EQ(0, std::memcmp(data1, ret[0].Data(), sizeof(data1)));
    EXPECT_EQ(sizeof(data1), ret[0].Size());
    EXPECT_EQ(0, std::memcmp(data2, ret[1].Data(), sizeof(data2)));
    EXPECT_EQ(sizeof(data2), ret[1].Size());
    EXPECT_EQ(0, std::memcmp(data3, ret[2].Data(), sizeof(data3)));
    EXPECT_EQ(sizeof(data3), ret[2].Size());
    EXPECT_EQ(0, std::memcmp(data4, ret[3].Data(), sizeof(data4)));
    EXPECT_EQ(sizeof(data4), ret[3].Size());
    EXPECT_EQ(0, std::memcmp(data5, ret[4].Data(), sizeof(data5)));
    EXPECT_EQ(sizeof(data5), ret[4].Size());
}

TEST(Byte, Split2)
{
    uint8_t data1[] = {0, 1, 2, 3, 4};
    uint8_t data2[] = {0, 1, 2, 3, 4, 5};
    uint8_t data3[] = {0, 1, 2, 3, 4, 5, 6};
    uint8_t data4[] = {0, 1, 2, 3, 4, 7, 8};
    uint8_t data5[] = {0, 1, 2, 3, 4, 89, 0, 9, 9};
    uint8_t sub[]   = {'t', 'e', 's', 't'};
    uint8_t data[sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(data5) + sizeof(sub) * 6];
    std::memcpy(data + sizeof(sub) * 0, sub, sizeof(sub));
    std::memcpy(data + sizeof(sub) * 1, data1, sizeof(data1));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 1, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 2, data2, sizeof(data2));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 2, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 3, data3, sizeof(data3));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(sub) * 3, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(sub) * 4, data4, sizeof(data4));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(sub) * 4, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(sub) * 5, data5, sizeof(data5));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(data3) + sizeof(data4) + sizeof(data5) + sizeof(sub) * 5, sub, sizeof(sub));

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 5);
    EXPECT_EQ(0, std::memcmp(data1, ret[0].Data(), sizeof(data1)));
    EXPECT_EQ(sizeof(data1), ret[0].Size());
    EXPECT_EQ(0, std::memcmp(data2, ret[1].Data(), sizeof(data2)));
    EXPECT_EQ(sizeof(data2), ret[1].Size());
    EXPECT_EQ(0, std::memcmp(data3, ret[2].Data(), sizeof(data3)));
    EXPECT_EQ(sizeof(data3), ret[2].Size());
    EXPECT_EQ(0, std::memcmp(data4, ret[3].Data(), sizeof(data4)));
    EXPECT_EQ(sizeof(data4), ret[3].Size());
    EXPECT_EQ(0, std::memcmp(data5, ret[4].Data(), sizeof(data5)));
    EXPECT_EQ(sizeof(data5), ret[4].Size());
}

TEST(Byte, Split3)
{
    uint8_t data1[] = {0, 1, 2, 3, 4};
    uint8_t data2[] = {0, 1, 2, 3, 4, 5};
    uint8_t sub[]   = {'t', 'e', 's', 't'};
    uint8_t data[sizeof(data1) + sizeof(data2) + sizeof(sub) * 3];
    std::memcpy(data, data1, sizeof(data1));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 0, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 1, sub, sizeof(sub));
    std::memcpy(data + sizeof(data1) + sizeof(sub) * 2, data2, sizeof(data2));
    std::memcpy(data + sizeof(data1) + sizeof(data2) + sizeof(sub) * 2, sub, sizeof(sub));

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 2);
    EXPECT_EQ(0, std::memcmp(data1, ret[0].Data(), sizeof(data1)));
    EXPECT_EQ(sizeof(data1), ret[0].Size());
    EXPECT_EQ(0, std::memcmp(data2, ret[1].Data(), sizeof(data2)));
    EXPECT_EQ(sizeof(data2), ret[1].Size());
}

TEST(Byte, Split4)
{
    uint8_t sub[] = {'t', 'e', 's', 't'};
    uint8_t data[sizeof(sub) * 3];
    for (size_t i = 0; i < 3; i++)
    {
        std::memcpy(data + i * sizeof(sub), sub, sizeof(sub));
    }

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 0);
}

TEST(Byte, Split5)
{
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    uint8_t data[] = {1, 2, 3};

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 1);
    EXPECT_EQ(0, std::memcmp(data, ret[0].Data(), sizeof(data)));
    EXPECT_EQ(sizeof(data), ret[0].Size());
}

TEST(Byte, Split6)
{
    uint8_t sub[]  = {'t', 'e', 's', 't'};
    uint8_t data[] = {1, 2, 3, 5, 6, 7, 8, 2, 34, 6, 1, 4, 3, 8, 9};

    auto ret = ByteSplit({data, sizeof(data)}, {sub, sizeof(sub)});
    ASSERT_EQ(ret.size(), 1);
    EXPECT_EQ(0, std::memcmp(data, ret[0].Data(), sizeof(data)));
    EXPECT_EQ(sizeof(data), ret[0].Size());
}

#ifdef WIN32
// windows的wchar_t是2字节和其他平台不同
// https://zh.cppreference.com/w/cpp/language/types
TEST(Byte, StrideSplit1)
{
    std::vector<std::wstring> datas;
    for (int i = 0; i < 5; i++)
    {
        datas.emplace_back(CharsetUtils::UTF8ToUnicode(RandWord()));
    }
    std::string data;
    for (int i = 0; i < 5; i++)
    {
        data.insert(data.end(), (char*) datas[i].data(), (char*) (datas[i].data() + datas[i].size()));
        data.push_back('\0');
        data.push_back('\0');
    }
    data.push_back('\0');
    data.push_back('\0');
    auto ret = ByteSplit({(uint8_t*) data.data(), data.size()}, {(uint8_t*) "\0\0", 2}, 2);
    ASSERT_EQ(ret.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(std::wstring((wchar_t*) ret[i].Data(), (wchar_t*) (ret[i].Data() + ret[i].Size())), datas[i]);
    }
}
#else
//非windows的wchar_t是4字节
TEST(Byte, StrideSplit1)
{
    std::vector<std::wstring> datas;
    for (int i = 0; i < 5; i++)
    {
        datas.emplace_back(CharsetUtils::UTF8ToUnicode(RandWord()));
    }
    std::string data;
    for (int i = 0; i < 5; i++)
    {
        data.insert(data.end(), (char*) datas[i].data(), (char*) (datas[i].data() + datas[i].size()));
        data.push_back('\0');
        data.push_back('\0');
        data.push_back('\0');
        data.push_back('\0');
    }

    auto ret = ByteSplit({(uint8_t*) data.data(), data.size()}, {(uint8_t*) "\0\0\0\0", 4}, 4);
    ASSERT_EQ(ret.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(std::wstring((wchar_t*) ret[i].Data(), (wchar_t*) (ret[i].Data() + ret[i].Size())), datas[i]);
    }
}
#endif

TEST(Byte, Join)
{
    uint8_t                     data1[] = {0, 1, 2, 3, 4};
    uint8_t                     data2[] = {0, 1, 2, 3, 4, 5};
    uint8_t                     data3[] = {'t', 'e', 's', 't'};
    std::vector<ByteBufferView> src     = {
        {data1, sizeof(data1)},
        {data2, sizeof(data2)},
        {data1, sizeof(data1)},
        {data2, sizeof(data2)}
    };
    auto ret = ByteJoin(src, {data3, sizeof(data3)});
    EXPECT_EQ(ret.size(), sizeof(data1) * 2 + sizeof(data2) * 2 + sizeof(data3) * 3);
    EXPECT_EQ(0, std::memcmp(ret.data(), data1, sizeof(data1)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1), data3, sizeof(data3)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(data3), data2, sizeof(data2)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(data3) + sizeof(data2), data3, sizeof(data3)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(data3) + sizeof(data2) + sizeof(data3), data1, sizeof(data1)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(data3) + sizeof(data2) + sizeof(data3) + sizeof(data1), data3, sizeof(data3)));
    EXPECT_EQ(
        0,
        std::memcmp(ret.data() + sizeof(data1) + sizeof(data3) + sizeof(data2) + sizeof(data3) + sizeof(data1) + sizeof(data3), data2, sizeof(data2))
    );
}

TEST(Byte, Replace)
{
    uint8_t                     data1[]   = {0, 1, 2, 3, 4};
    uint8_t                     data2[]   = {0, 1, 2, 3, 4, 5};
    uint8_t                     data3[]   = {'t', 'e', 's', 't'};
    uint8_t                     replace[] = {'r', 'e', 'p', 'l', 'a', 'c', 'e'};
    std::vector<ByteBufferView> src       = {
        {data1, sizeof(data1)},
        {data2, sizeof(data2)},
        {data1, sizeof(data1)},
        {data2, sizeof(data2)}
    };
    auto join = ByteJoin(src, {data3, sizeof(data3)});
    auto ret  = ByteReplace({join.data(), join.size()}, {data3, sizeof(data3)}, {replace, sizeof(replace)});
    EXPECT_EQ(ret.size(), sizeof(data1) * 2 + sizeof(data2) * 2 + sizeof(replace) * 3);
    EXPECT_EQ(0, std::memcmp(ret.data(), data1, sizeof(data1)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1), replace, sizeof(replace)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace), data2, sizeof(data2)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2), replace, sizeof(replace)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2) + sizeof(replace), data1, sizeof(data1)));
    EXPECT_EQ(
        0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2) + sizeof(replace) + sizeof(data1), replace, sizeof(replace))
    );
    EXPECT_EQ(
        0, std::memcmp(
               ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2) + sizeof(replace) + sizeof(data1) + sizeof(replace), data2, sizeof(data2)
           )
    );
}

TEST(Byte, Replace1)
{
    uint8_t                     data1[]   = {0, 1, 2, 3, 4};
    uint8_t                     data2[]   = {0, 1, 2, 3, 4, 5};
    uint8_t                     data3[]   = {'t', 'e', 's', 't'};
    uint8_t                     replace[] = {'r', 'e', 'p'};
    std::vector<ByteBufferView> src       = {
        {data1, sizeof(data1)},
        {data2, sizeof(data2)},
        {data1, sizeof(data1)},
        {data2, sizeof(data2)}
    };
    auto join = ByteJoin(src, {data3, sizeof(data3)});
    auto ret  = ByteReplace({join.data(), join.size()}, {data3, sizeof(data3)}, {replace, sizeof(replace)});
    EXPECT_EQ(ret.size(), sizeof(data1) * 2 + sizeof(data2) * 2 + sizeof(replace) * 3);
    EXPECT_EQ(0, std::memcmp(ret.data(), data1, sizeof(data1)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1), replace, sizeof(replace)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace), data2, sizeof(data2)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2), replace, sizeof(replace)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2) + sizeof(replace), data1, sizeof(data1)));
    EXPECT_EQ(
        0, std::memcmp(ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2) + sizeof(replace) + sizeof(data1), replace, sizeof(replace))
    );
    EXPECT_EQ(
        0, std::memcmp(
               ret.data() + sizeof(data1) + sizeof(replace) + sizeof(data2) + sizeof(replace) + sizeof(data1) + sizeof(replace), data2, sizeof(data2)
           )
    );
}

TEST(Byte, Replace2)
{
    uint8_t                     data1[] = {0, 1, 2, 3, 4};
    uint8_t                     data2[] = {0, 1, 2, 3, 4, 5};
    uint8_t                     data3[] = {'t', 'e', 's', 't'};
    std::vector<ByteBufferView> src     = {
        {data1, sizeof(data1)},
        {data2, sizeof(data2)},
        {data1, sizeof(data1)},
        {data2, sizeof(data2)}
    };
    auto join = ByteJoin(src, {data3, sizeof(data3)});
    auto ret  = ByteReplace({join.data(), join.size()}, {data3, sizeof(data3)}, {nullptr, 0});
    EXPECT_EQ(ret.size(), sizeof(data1) * 2 + sizeof(data2) * 2 + 0 * 3);
    EXPECT_EQ(0, std::memcmp(ret.data(), data1, sizeof(data1)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + 0, data2, sizeof(data2)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + 0 + sizeof(data2) + 0, data1, sizeof(data1)));
    EXPECT_EQ(0, std::memcmp(ret.data() + sizeof(data1) + 0 + sizeof(data2) + 0 + sizeof(data1) + 0, data2, sizeof(data2)));
}

TEST(Byte, StartWith)
{
    uint8_t data1[] = {'t', 'e', 's', 't', 0, 1, 2, 3, 4};
    uint8_t data2[] = {0, 1, 2, 3, 4, 't', 'e', 's', 't', 0, 1, 2, 3, 4};
    uint8_t sub[]   = {'t', 'e', 's', 't'};
    EXPECT_TRUE(ByteStartWith({data1, sizeof(data1)}, {sub, sizeof(sub)}));
    EXPECT_FALSE(ByteStartWith({data2, sizeof(data2)}, {sub, sizeof(sub)}));
}

TEST(Byte, EndWith)
{
    uint8_t data1[] = {0, 0, 1, 2, 3, 4, 't', 'e', 's', 't'};
    uint8_t data2[] = {0, 1, 2, 3, 4, 't', 'e', 's', 't', 0, 1, 2, 3, 4};
    uint8_t sub[]   = {'t', 'e', 's', 't'};
    EXPECT_TRUE(ByteEndWith({data1, sizeof(data1)}, {sub, sizeof(sub)}));
    EXPECT_FALSE(ByteEndWith({data2, sizeof(data2)}, {sub, sizeof(sub)}));
}

TEST(Byte, HexTobyte)
{
    uint8_t data[100];
    RandBytes(data, sizeof(data));
    {
        auto hex   = BytesToHexString(data, sizeof(data), true);
        auto bytes = HexStringToBytes(hex);
        ASSERT_TRUE(bytes);
        EXPECT_EQ(0, std::memcmp(data, bytes->data(), sizeof(data)));
    }
    {
        auto hex   = BytesToHexString(data, sizeof(data), false);
        auto bytes = HexStringToBytes(hex);
        ASSERT_TRUE(bytes);
        EXPECT_EQ(0, std::memcmp(data, bytes->data(), sizeof(data)));
    }
}

TEST(Byte, Count)
{
    uint32_t num = 0b11111111111111110000000000000000;
    EXPECT_EQ(CountLeftZero(num), 0);
    EXPECT_EQ(CountLeftZero(~num), 16);
    EXPECT_EQ(CountLeftZero(0u), 32);
    EXPECT_EQ(CountLeftZero(1u), 31);
    EXPECT_EQ(CountLeftZero(0xFFFFFFFF), 0);

    EXPECT_EQ(CountLeftOne(num), 16);
    EXPECT_EQ(CountLeftOne(~num), 0);
    EXPECT_EQ(CountLeftOne(0u), 0);
    EXPECT_EQ(CountLeftOne(1u), 0);
    EXPECT_EQ(CountLeftOne(0xFFFFFFFF), 32);

    EXPECT_EQ(CountRightZero(num), 16);
    EXPECT_EQ(CountRightZero(~num), 0);
    EXPECT_EQ(CountRightZero(0u), 32);
    EXPECT_EQ(CountRightZero(1u), 0);
    EXPECT_EQ(CountRightZero(0xFFFFFFFF), 0);

    EXPECT_EQ(CountRightOne(num), 0);
    EXPECT_EQ(CountRightOne(~num), 16);
    EXPECT_EQ(CountRightOne(0u), 0);
    EXPECT_EQ(CountRightOne(1u), 1);
    EXPECT_EQ(CountRightOne(0xFFFFFFFF), 32);

    EXPECT_EQ(CountLeftZero(static_cast<uint8_t>(0)), 8);
    EXPECT_EQ(CountLeftZero(static_cast<uint8_t>(1)), 7);
    EXPECT_EQ(CountLeftOne(static_cast<uint8_t>(0)), 0);
    EXPECT_EQ(CountLeftOne(static_cast<uint8_t>(1)), 0);
    EXPECT_EQ(CountRightZero(static_cast<uint8_t>(0)), 8);
    EXPECT_EQ(CountRightZero(static_cast<uint8_t>(1)), 0);
    EXPECT_EQ(CountRightOne(static_cast<uint8_t>(0)), 0);
    EXPECT_EQ(CountRightOne(static_cast<uint8_t>(1)), 1);

    EXPECT_EQ(CountZero(num), 16);
    EXPECT_EQ(CountZero(~num), 16);
    EXPECT_EQ(CountZero(0u), 32);
    EXPECT_EQ(CountZero(1u), 31);
    EXPECT_EQ(CountZero(0xFFFFFFFF), 0);
    EXPECT_EQ(CountOne(num), 16);
    EXPECT_EQ(CountOne(~num), 16);
    EXPECT_EQ(CountOne(0u), 0);
    EXPECT_EQ(CountOne(1u), 1);
    EXPECT_EQ(CountOne(0xFFFFFFFF), 32);
    EXPECT_EQ(CountOne(static_cast<uint8_t>(0)), 0);
    EXPECT_EQ(CountOne(static_cast<uint8_t>(1)), 1);
    EXPECT_EQ(CountZero(static_cast<uint8_t>(0)), 8);
    EXPECT_EQ(CountZero(static_cast<uint8_t>(1)), 7);

    std::array<uint32_t, 4> bytes = {0b11111111111111110000000000000000, 0b00000000000000000000000000000000, 0b11111111111111111111111111111111, 1};
    EXPECT_EQ(CountOne({reinterpret_cast<const uint8_t*>(bytes.data()), 16}), 49);
    EXPECT_EQ(CountZero({reinterpret_cast<const uint8_t*>(bytes.data()), 16}), 79);
}

TEST(Bytes, FlipBytes16)
{
    EXPECT_EQ((uint16_t) 0x3412, FlipBytes((uint16_t) 0x1234));
}

TEST(Bytes, FlipBytes32)
{
    EXPECT_EQ((uint32_t) 0x78563412, FlipBytes((uint32_t) 0x12345678));
}

TEST(Bytes, FlipBytes64)
{
    EXPECT_EQ((uint64_t) 0x2143658778563412, FlipBytes((uint64_t) 0x1234567887654321));
}