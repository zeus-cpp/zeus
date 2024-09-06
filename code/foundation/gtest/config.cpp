#include <any>
#include <random>
#include <map>
#include <list>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/crypt/md5_digest.h>
#include <zeus/foundation/crypt/hmac_digest.h>
#include <zeus/foundation/crypt/aes_encrypt.h>
#include <zeus/foundation/crypt/aes_decrypt.h>
#include <zeus/foundation/config/general_config.h>
#include <zeus/foundation/config/switch_config.h>
#include <zeus/foundation/config/backup_config.h>
#include <zeus/foundation/config/layered_config.h>
#include <zeus/foundation/config/config_view.h>
#include "zeus/foundation/container/container_cast.hpp"
#include <zeus/foundation/serialization/serializer.h>
#include <zeus/foundation/serialization/file_serializer.h>
#include <zeus/foundation/string/string_utils.h>

using namespace zeus;

namespace
{
std::string kDelim(".");

std::any GenerateAny(int type)
{
    switch (type)
    {
    case 0:
        return zeus::RandWord();
    case 1:
        return (int) zeus::RandUint32();
    case 2:
        return (unsigned int) zeus::RandUint32();
    case 3:
        return (long long) zeus::RandUint64();
    case 4:
        return (unsigned long long) zeus::RandUint64();
    case 5:
        return zeus::RandDouble();
    case 6:
        return zeus::RandBool();
    default:
        assert(false);
        return zeus::RandBool();
    }
}

std::any RandomAny(bool withoutBool = false)
{
    return GenerateAny(RandUint8(withoutBool ? 5 : 6));
}

std::vector<std::any> RandomAnyArray(bool withoutBool = false)
{
    std::vector<std::any> result;
    result.reserve(10);
    auto type = RandUint8(withoutBool ? 5 : 6);
    for (int i = 0; i < 10; i++)
    {
        result.emplace_back(GenerateAny(type));
    }
    return result;
}

std::map<std::string, std::any> GenerateRandomMap(bool withoutBool = false)
{
    std::map<std::string, std::any> result;
    for (int i = 0; i <= 30; i++)
    {
        result[RandWord() + kDelim + RandWord() + kDelim + RandWord()] = RandomAny(withoutBool);
    }
    return result;
}

void GenerateRandomMap(std::map<std::string, std::any> &data, bool withoutBool = false)
{
    for (auto &element : data)
    {
        element.second = RandomAny(withoutBool);
    }
}

std::map<std::string, std::vector<std::any>> GenerateRandomArrayMap(bool withoutBool = false)
{
    std::map<std::string, std::vector<std::any>> result;
    for (int i = 0; i <= 30; i++)
    {
        result[RandWord() + kDelim + RandWord() + kDelim + RandWord()] = RandomAnyArray(withoutBool);
    }
    return result;
}

void GenerateRandomArrayMap(std::map<std::string, std::vector<std::any>> &data, bool withoutBool = false)
{
    for (auto &element : data)
    {
        element.second = RandomAnyArray(withoutBool);
    }
}

std::map<std::string, std::string> GenerateRandomCircleMap()
{
    std::map<std::string, std::string> result;
    for (int i = 0; i <= 30; i++)
    {
        result[RandWord() + kDelim + RandWord()] = RandWord();
    }
    return result;
}

void SetConfig(const std::pair<std::string, std::any> &element, ConfigView &config)
{
    if (element.second.type() == typeid(int))
    {
        config.Set(element.first, std::any_cast<int>(element.second));
    }
    if (element.second.type() == typeid(unsigned int))
    {
        config.Set(element.first, std::any_cast<unsigned int>(element.second));
    }
    if (element.second.type() == typeid(long long))
    {
        config.Set(element.first, std::any_cast<long long>(element.second));
    }
    if (element.second.type() == typeid(unsigned long long))
    {
        config.Set(element.first, std::any_cast<unsigned long long>(element.second));
    }
    if (element.second.type() == typeid(bool))
    {
        config.Set(element.first, std::any_cast<bool>(element.second));
    }
    if (element.second.type() == typeid(double))
    {
        config.Set(element.first, std::any_cast<double>(element.second));
    }
    if (element.second.type() == typeid(std::string))
    {
        config.Set(element.first, std::any_cast<std::string>(element.second));
    }
}

void SetConfig(const std::map<std::string, std::any> &data, ConfigView &config)
{
    for (const auto &element : data)
    {
        SetConfig(element, config);
    }
}

void CheckConfig(const std::pair<std::string, std::any> &element, const ConfigView &config)
{
    if (element.second.type() == typeid(int))
    {
        EXPECT_EQ(std::any_cast<int>(element.second), config.Get<int>(element.first));
    }
    if (element.second.type() == typeid(unsigned int))
    {
        EXPECT_EQ(std::any_cast<unsigned int>(element.second), config.Get<unsigned int>(element.first));
    }
    if (element.second.type() == typeid(long long))
    {
        EXPECT_EQ(std::any_cast<long long>(element.second), config.Get<long long>(element.first));
    }
    if (element.second.type() == typeid(unsigned long long))
    {
        EXPECT_EQ(std::any_cast<unsigned long long>(element.second), config.Get<unsigned long long>(element.first));
    }
    if (element.second.type() == typeid(bool))
    {
        EXPECT_EQ(std::any_cast<bool>(element.second), config.Get<bool>(element.first));
    }
    if (element.second.type() == typeid(double))
    {
        EXPECT_EQ(std::any_cast<double>(element.second), config.Get<double>(element.first));
    }
    if (element.second.type() == typeid(std::string))
    {
        EXPECT_EQ(std::any_cast<std::string>(element.second), config.Get<std::string>(element.first));
    }
}

void CheckConfig(const std::map<std::string, std::any> &data, const ConfigView &config)
{
    for (auto &element : data)
    {
        CheckConfig(element, config);
    }
}

void SetConfigArray(const std::pair<std::string, std::vector<std::any>> &element, ConfigView &config)
{
    if (element.second[0].type() == typeid(int))
    {
        std::vector<int> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<int>(v));
        }
        config.Set(element.first, arrayData);
    }
    if (element.second[0].type() == typeid(unsigned int))
    {
        std::vector<unsigned int> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<unsigned int>(v));
        }
        config.Set(element.first, arrayData);
    }
    if (element.second[0].type() == typeid(long long))
    {
        std::vector<long long> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<long long>(v));
        }
        config.Set(element.first, arrayData);
    }
    if (element.second[0].type() == typeid(unsigned long long))
    {
        std::vector<unsigned long long> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<unsigned long long>(v));
        }
        config.Set(element.first, arrayData);
    }
    if (element.second[0].type() == typeid(bool))
    {
        std::vector<bool> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<bool>(v));
        }
        config.Set(element.first, arrayData);
    }
    if (element.second[0].type() == typeid(double))
    {
        std::vector<double> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<double>(v));
        }
        config.Set(element.first, arrayData);
    }
    if (element.second[0].type() == typeid(std::string))
    {
        std::vector<std::string> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<std::string>(v));
        }
        config.Set(element.first, arrayData);
    }
}

void SetConfigArray(const std::map<std::string, std::vector<std::any>> &data, ConfigView &config)
{
    for (const auto &element : data)
    {
        SetConfigArray(element, config);
    }
}

void CheckConfigArray(const std::pair<std::string, std::vector<std::any>> &element, ConfigView &config)
{
    if (element.second[0].type() == typeid(int))
    {
        std::vector<int> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<int>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<int>>(element.first));
    }
    if (element.second[0].type() == typeid(unsigned int))
    {
        std::vector<unsigned int> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<unsigned int>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<unsigned int>>(element.first));
    }
    if (element.second[0].type() == typeid(long long))
    {
        std::vector<long long> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<long long>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<long long>>(element.first));
    }
    if (element.second[0].type() == typeid(unsigned long long))
    {
        std::vector<unsigned long long> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<unsigned long long>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<unsigned long long>>(element.first));
    }
    if (element.second[0].type() == typeid(bool))
    {
        std::vector<bool> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<bool>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<bool>>(element.first));
    }
    if (element.second[0].type() == typeid(double))
    {
        std::vector<double> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<double>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<double>>(element.first));
    }
    if (element.second[0].type() == typeid(std::string))
    {
        std::vector<std::string> arrayData;
        for (auto &v : element.second)
        {
            arrayData.emplace_back(std::any_cast<std::string>(v));
        }
        EXPECT_EQ(arrayData, config.Get<std::vector<std::string>>(element.first));
    }
}

void CheckConfigArray(const std::map<std::string, std::vector<std::any>> &data, const ConfigView &config)
{
    for (const auto &element : data)
    {
        CheckConfig(element, config);
    }
}

void SetCircle(const std::pair<std::string, std::string> &element, ConfigView &config)
{
    config.Set(element.first, element.second);
    config.Set(element.first + "CircleCircleCircle", "Circle${" + element.first + "}Circle");
}

void SetCircle(const std::map<std::string, std::string> &data, ConfigView &config)
{
    for (const auto &element : data)
    {
        SetCircle(element, config);
    }
}

void CheckCircle(const std::pair<std::string, std::string> &element, const ConfigView &config)
{
    EXPECT_TRUE(element.second == config.Get<std::string>(element.first));
    auto value = config.Get<std::string>(element.first + "CircleCircleCircle").value();
    EXPECT_TRUE("Circle" + element.second + "Circle" == config.ExpandValue(value).value());
}

void CheckCircle(const std::map<std::string, std::string> &data, const ConfigView &config)
{
    for (const auto &element : data)
    {
        CheckCircle(element, config);
    }
}

void ConfigRandomTest(ConfigView &config, const std::function<void()> &serialization = nullptr, bool subkey = true)
{
    auto data       = GenerateRandomMap();
    auto arraydata  = GenerateRandomArrayMap();
    auto circleData = GenerateRandomCircleMap();
    SetConfig(data, config);
    SetConfigArray(arraydata, config);
    SetCircle(circleData, config);
    CheckConfig(data, config);
    CheckConfigArray(arraydata, config);
    CheckCircle(circleData, config);
    if (subkey)
    {
        auto                  keys = VectorToSet(config.Keys(""));
        std::set<std::string> randKeys;
        for (const auto &item : data)
        {
            randKeys.emplace(Split(item.first, kDelim).at(0));
        }
        for (const auto &item : arraydata)
        {
            randKeys.emplace(Split(item.first, kDelim).at(0));
        }
        for (const auto &item : circleData)
        {
            randKeys.emplace(Split(item.first, kDelim).at(0));
        }
        EXPECT_EQ(keys, randKeys);
    }
    GenerateRandomMap(data);
    GenerateRandomArrayMap(arraydata);
    SetConfig(data, config);
    SetConfigArray(arraydata, config);
    CheckConfig(data, config);
    CheckConfigArray(arraydata, config);

    config.Remove(data.begin()->first);
    EXPECT_FALSE(config.Has(data.begin()->first));
    data.erase(data.begin());

    if (serialization)
    {
        serialization();
    }

    CheckConfig(data, config);
    CheckConfigArray(arraydata, config);
    CheckCircle(circleData, config);
}

} // namespace

TEST(Config, View)
{
    GeneralConfig  config;
    nlohmann::json data = {
        {"test", {{"a", "dassdsa"}, {"b", 2}, {"c", 3}}}
    };
    config.SetConfigMap(data);
    ConfigView view(config, "test");
    EXPECT_TRUE(view.Has(""));
    EXPECT_TRUE(view.Has("a"));
    EXPECT_TRUE(view.Has("b"));
    EXPECT_FALSE(view.Has("d"));
}

TEST(Config, Value)
{
    GeneralConfig  config;
    nlohmann::json data = {
        {"test", {{"a", "dassdsa"}, {"b", 2}, {"c", 3}}}
    };
    config.SetConfigMap(data);
    ConfigView view(config);
    EXPECT_TRUE(view.Has("/test"));
    EXPECT_TRUE(view.Has("test/a"));
    EXPECT_TRUE(view.Has("test.b"));
    EXPECT_FALSE(view.Has("test.d"));
    EXPECT_EQ("dassdsa", view.Get<std::string>("test/a"));
    EXPECT_EQ(3, view.Get<size_t>("test/c"));
    EXPECT_TRUE(view.Set("test/a", "test"));
    EXPECT_TRUE(view.Set("test/d/f/g", "testdfg"));
    EXPECT_EQ("testdfg", view.Get<std::string>("test/d/f/g"));
    EXPECT_EQ("test", view.Get<std::string>("test/a"));
    EXPECT_NE("dassdsa", view.Get<std::string>("test/a"));
    EXPECT_EQ("56789", view.Get<std::string>("test/8", "56789"));
    EXPECT_EQ(3, view.Get<int>("test/c", 89));
    std::string temp;
    EXPECT_TRUE(view.GetTo("test/a", temp));
    EXPECT_EQ("test", temp);
    EXPECT_FALSE(view.GetTo("test/8", temp));
    EXPECT_TRUE(view.Remove("test.b"));
    EXPECT_FALSE(view.Remove("test.not"));
    EXPECT_FALSE(view.Has("test.b"));
}

TEST(Config, Array)
{
    GeneralConfig  config;
    nlohmann::json data = {
        {"test", {{"string_array", {"13", "24", "35"}}, {"int_array", {5, 6, 7}}}}
    };
    config.SetConfigMap(data);
    ConfigView view(config);
    EXPECT_TRUE(view.Has("/test/string_array"));
    EXPECT_TRUE(view.Has("test.int_array"));
    EXPECT_TRUE(view.Has("test/string_array/2"));
    EXPECT_TRUE(view.Has("test.int_array.1"));
    EXPECT_FALSE(view.Has("test.string_array.13"));
    std::vector<std::string> stringArray {"13", "24", "35"};
    EXPECT_EQ(stringArray, view.Get<std::vector<std::string>>("test.string_array"));
    std::vector<int> intArray {5, 6, 7};
    EXPECT_EQ(intArray, view.Get<std::vector<int>>("test/int_array"));
    stringArray = {"gdsf", "wwq", "d3d"};
    intArray    = {9, 0, 0};
    EXPECT_TRUE(view.Set("test/d/f/int", intArray));
    EXPECT_TRUE(view.Set("test.string_array", stringArray));
    EXPECT_EQ(intArray, view.Get<std::vector<int>>("test/d/f/int"));
    EXPECT_EQ(stringArray, view.Get<std::vector<std::string>>("test.string_array"));
    EXPECT_EQ(intArray, view.Get<decltype(intArray)>("test/8", intArray));
    EXPECT_EQ(stringArray, view.Get<decltype(stringArray)>("test/c", stringArray));
    std::vector<std::string> temp;
    EXPECT_TRUE(view.GetTo("test/string_array", temp));
    EXPECT_EQ(stringArray, temp);
    EXPECT_FALSE(view.GetTo("test/8", temp));
    EXPECT_TRUE(view.Remove("test.string_array"));
    EXPECT_FALSE(view.Remove("test.not"));
    EXPECT_FALSE(view.Has("test.string_array"));
}

struct TestObject
{
    bool operator==(const TestObject &other) const
    {
        return boolData == other.boolData && stringData == other.stringData && listData == other.listData;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TestObject, boolData, stringData, listData)

    bool           boolData = false;
    std::string    stringData;
    std::list<int> listData;
};

TEST(Config, Object)
{
    GeneralConfig config;
    ConfigView    view(config);
    TestObject    object;
    object.boolData   = true;
    object.stringData = "test";
    object.listData   = {1, 2, 3};
    EXPECT_TRUE(view.Set("test/a", object));
    EXPECT_TRUE(view.Has("/test/a"));
    EXPECT_TRUE(view.Has("test.a"));
    EXPECT_FALSE(view.Has("test.b"));
    EXPECT_TRUE(view.Has("/test/a/boolData"));
    EXPECT_TRUE(view.Has("test.a.boolData"));
    EXPECT_EQ(object, view.Get<TestObject>("test/a"));
    EXPECT_EQ(object.listData, view.Get<decltype(object.listData)>("test/a/listData"));

    EXPECT_TRUE(view.Set("", object));
    EXPECT_FALSE(view.Has("/test/a"));
    EXPECT_FALSE(view.Has("test.a"));
    EXPECT_FALSE(view.Has("test.b"));
    EXPECT_FALSE(view.Has("/test/a/boolData"));
    EXPECT_FALSE(view.Has("test.a.boolData"));
    EXPECT_TRUE(view.Has("/boolData"));
    EXPECT_TRUE(view.Has("boolData"));
    EXPECT_EQ(object, view.Get<TestObject>(""));
    EXPECT_EQ(object.listData, view.Get<decltype(object.listData)>("listData"));

    EXPECT_TRUE(view.Set("", object));
    EXPECT_FALSE(view.Has("/test/a"));
    EXPECT_FALSE(view.Has("test.a"));
    EXPECT_FALSE(view.Has("test.b"));
    EXPECT_FALSE(view.Has("/test/a/boolData"));
    EXPECT_FALSE(view.Has("test.a.boolData"));
    EXPECT_TRUE(view.Has("/boolData"));
    EXPECT_TRUE(view.Has("boolData"));
    EXPECT_EQ(object, view.Get<TestObject>(""));
    EXPECT_EQ(object.listData, view.Get<decltype(object.listData)>("listData"));

    EXPECT_TRUE(view.Remove(""));
    EXPECT_FALSE(view.Has("/test/a"));
    EXPECT_FALSE(view.Has("test.a"));
    EXPECT_FALSE(view.Has("test.b"));
    EXPECT_FALSE(view.Has("/test/a/boolData"));
    EXPECT_FALSE(view.Has("test.a.boolData"));
    EXPECT_FALSE(view.Has("/boolData"));
    EXPECT_FALSE(view.Has("boolData"));
    EXPECT_TRUE(view.Keys("").empty());
    EXPECT_TRUE(view.Keys("/").empty());
}

TEST(Config, RandomValue)
{
    GeneralConfig config;
    ConfigView    view(config);
    ConfigRandomTest(view);
}

TEST(Config, Multithread)
{
    GeneralConfig            config;
    ConfigView               view(config);
    std::vector<std::thread> pool;
    for (int i = 0; i < 5; i++)
    {
        pool.emplace_back(std::thread([&]() { ConfigRandomTest(view, nullptr, false); }));
    }
    for (auto &thead : pool)
    {
        thead.join();
    }
}

TEST(Config, Notify)
{
    size_t        key1Count = 0;
    size_t        key2Count = 0;
    GeneralConfig config;
    ConfigView    view(config);
    std::string   key1Value;
    std::string   key2Value;
    auto          key1 = RandWord();
    auto          key2 = RandWord();
    auto          key3 = RandWord();
    auto          id1  = config.AddChangeNotify(
        key1,
        [&key1Count, &key1Value](const std::string &value)
        {
            key1Count++;
            EXPECT_EQ(key1Value, value);
        }
    );
    auto id2 = config.AddChangeNotify(
        key2,
        [&key2Count, &key2Value](const std::string &value)
        {
            key2Count++;
            EXPECT_EQ(key2Value, value);
        }
    );

    auto arrayData = GenerateRandomArrayMap();
    view.Set(key1, key1Value);
    key1Value = RandWord();
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    key1Value = RandWord();
    view.Set(key1, key1Value);
    EXPECT_EQ(key1Count, 3);
    EXPECT_EQ(key2Count, 0);
    config.AddChangeNotify(
        key1,
        [&key1Count, &key1Value](const std::string &value)
        {
            key1Count++;
            EXPECT_EQ(key1Value, value);
        }
    );
    key1Value = RandWord();
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    view.Set(key1, key1Value);
    key1Value = RandWord();
    view.Set(key1, key1Value);
    EXPECT_EQ(key1Count, 7);
    EXPECT_EQ(key2Count, 0);
    config.RemoveChangeNotify(id1);
    key1Value = RandWord();
    view.Set(key1, key1Value);
    key1Value = RandWord();
    view.Set(key1, key1Value);
    EXPECT_EQ(key1Count, 9);
    EXPECT_EQ(key2Count, 0);

    key2Value = RandWord();
    view.Set(key2, key2Value);
    key2Value = RandWord();
    view.Set(key2, key2Value);
    EXPECT_EQ(key1Count, 9);
    EXPECT_EQ(key2Count, 2);
}

TEST(Config, SwitchConfig)
{
    SwitchConfig switchConfig("release");

    auto       devAConfig    = std::make_shared<GeneralConfig>();
    auto       devBConfig    = std::make_shared<GeneralConfig>();
    auto       devConfig     = std::make_shared<GeneralConfig>();
    auto       releaseConfig = std::make_shared<GeneralConfig>();
    ConfigView devAConfigView(*devAConfig);
    ConfigView devBConfigView(*devBConfig);
    ConfigView devConfigView(*devConfig);
    ConfigView releaseConfigView(*releaseConfig);
    auto       devAData = GenerateRandomMap();
    auto       devBData = devAData;
    GenerateRandomMap(devBData);

    auto releaseData = devAData;
    GenerateRandomMap(releaseData);

    auto devData = GenerateRandomMap();

    SetConfig(devAData, devAConfigView);
    SetConfig(devBData, devBConfigView);
    SetConfig(devData, devConfigView);
    SetConfig(releaseData, releaseConfigView);
    switchConfig.AddConfig(devAConfig, "dev-a");
    switchConfig.AddConfig(devBConfig, "dev-b");
    switchConfig.AddConfig(devConfig, "dev");
    switchConfig.AddConfig(releaseConfig, "release");
    CheckConfig(devAData, *devAConfig);
    CheckConfig(devBData, *devBConfig);
    CheckConfig(releaseData, *releaseConfig);

    switchConfig.Switch("dev-a");
    CheckConfig(devAData, switchConfig);

    switchConfig.Switch("dev-b");
    CheckConfig(devBData, switchConfig);

    switchConfig.Switch("hghf");
    CheckConfig(releaseData, switchConfig);
}

TEST(Config, BackupConfig)
{
    BackupConfig backupConfig;

    auto       config1 = std::make_shared<GeneralConfig>();
    auto       config2 = std::make_shared<GeneralConfig>();
    ConfigView config1View(*config1);
    ConfigView config2View(*config2);
    auto       data1 = GenerateRandomMap();
    auto       data2 = data1;
    GenerateRandomMap(data2);

    backupConfig.AddConfig(config1, "config1", 0);
    backupConfig.AddConfig(config2, "config2", -1);
    ConfigView backView(backupConfig);

    SetConfig(data1, backView);
    CheckConfig(data1, *config1);
    CheckConfig(data1, *config2);
    CheckConfig(data1, backupConfig);
    SetConfig(data2, config2View);
    CheckConfig(data2, backupConfig);
}

TEST(Config, Layer)
{
    LayeredConfig layerConfig;

    auto       config1 = std::make_shared<GeneralConfig>();
    auto       config2 = std::make_shared<GeneralConfig>();
    ConfigView config1View(*config1);
    ConfigView config2View(*config2);
    auto       data1 = GenerateRandomMap();
    auto       data2 = data1;
    GenerateRandomMap(data2);

    SetConfig(data1, config1View);
    SetConfig(data2, config2View);
    layerConfig.AddConfig(config1, "config1", false, 0);
    layerConfig.AddConfig(config2, "config2", false, -1);
    ConfigView layerView(layerConfig);

    CheckConfig(data1, *config1);
    CheckConfig(data2, *config2);

    CheckConfig(data2, layerConfig);

    CheckConfig(data1, *layerConfig.FindConfig("config1"));
    CheckConfig(data2, *layerConfig.FindConfig("config2"));

    SetConfig(data2, layerView);

    auto config3 = std::make_shared<GeneralConfig>();
    auto data3   = data1;
    GenerateRandomMap(data3);
    layerConfig.AddConfig(config3, "config3", true, 10);
    SetConfig(data3, layerView);
    CheckConfig(data2, layerConfig);
    CheckConfig(data3, *layerConfig.FindConfig("config3"));
}

TEST(Config, LayerNotify)
{
#define CHECK_VALUE(Type, ids)                                                     \
    if (element.second.type() == typeid(Type))                                     \
    {                                                                              \
        ids.emplace_back(layerConfig.AddChangeNotify(                              \
            element.first,                                                         \
            [element, &notifyCount](const ConfigValue &value)                      \
            {                                                                      \
                EXPECT_EQ(value.get<Type>(), std::any_cast<Type>(element.second)); \
                notifyCount++;                                                     \
            }                                                                      \
        ));                                                                        \
    }

#define CHECK_ARRAY_VALUE(Type, ids)                                  \
    if (element.second[0].type() == typeid(Type))                     \
    {                                                                 \
        ids.emplace_back(layerConfig.AddChangeNotify(                 \
            element.first,                                            \
            [element, &notifyArrayCount](const ConfigValue &value)    \
            {                                                         \
                std::vector<Type> arrayData;                          \
                for (auto &v : element.second)                        \
                {                                                     \
                    arrayData.emplace_back(std::any_cast<Type>(v));   \
                }                                                     \
                EXPECT_EQ(value.get<std::vector<Type>>(), arrayData); \
                notifyArrayCount++;                                   \
            }                                                         \
        ));                                                           \
    }

    constexpr bool kGenerateWithoutBool = true; // bool 容易撞车，导致数据没有变化

    LayeredConfig layerConfig;

    auto       config1 = std::make_shared<GeneralConfig>();
    auto       config2 = std::make_shared<GeneralConfig>();
    ConfigView config1View(*config1);
    ConfigView config2View(*config2);
    auto       data1 = GenerateRandomMap(kGenerateWithoutBool);
    auto       data2 = data1;
    GenerateRandomMap(data2, kGenerateWithoutBool);
    auto data3 = data1;
    GenerateRandomMap(data3, kGenerateWithoutBool);
    auto arrayData = GenerateRandomArrayMap(kGenerateWithoutBool);

    layerConfig.AddConfig(config1, "config1", true, 0);
    layerConfig.AddConfig(config2, "config2", true, -1);
    ConfigView layerView(layerConfig);

    size_t            notifyCount      = 0;
    size_t            notifyArrayCount = 0;
    std::list<size_t> dataCallbackIds;
    std::list<size_t> arrayDataCallbackIds;
    for (const auto &element : data1)
    {
        CHECK_VALUE(int, dataCallbackIds);
        CHECK_VALUE(unsigned int, dataCallbackIds);
        CHECK_VALUE(long long, dataCallbackIds);
        CHECK_VALUE(unsigned long long, dataCallbackIds);
        CHECK_VALUE(bool, dataCallbackIds);
        CHECK_VALUE(double, dataCallbackIds);
        CHECK_VALUE(std::string, dataCallbackIds);
    }
    for (const auto &element : arrayData)
    {
        CHECK_ARRAY_VALUE(int, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(unsigned int, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(long long, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(unsigned long long, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(bool, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(double, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(std::string, arrayDataCallbackIds);
    }
    // NOTE: config1的优先级排序靠后，所以不会触发修改回调
    SetConfigArray(arrayData, layerView);
    EXPECT_EQ(notifyArrayCount, arrayData.size() * 1);
    SetConfigArray(arrayData, layerView);
    EXPECT_EQ(notifyArrayCount, arrayData.size() * 1); // 数据无变化
    GenerateRandomArrayMap(arrayData, kGenerateWithoutBool);
    for (const auto &id : arrayDataCallbackIds)
    {
        layerConfig.RemoveChangeNotify(id);
    }
    arrayDataCallbackIds.clear();
    for (const auto &element : arrayData)
    {
        CHECK_ARRAY_VALUE(int, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(unsigned int, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(long long, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(unsigned long long, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(bool, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(double, arrayDataCallbackIds);
        CHECK_ARRAY_VALUE(std::string, arrayDataCallbackIds);
    }
    SetConfigArray(arrayData, layerView);
    EXPECT_EQ(notifyArrayCount, arrayData.size() * 2);
    SetConfig(data1, layerView);
    EXPECT_EQ(notifyCount, data1.size() * 1);
    SetConfig(data1, config2View);
    EXPECT_EQ(notifyCount, data1.size() * 1); // 数据无变化
    for (const auto &id : dataCallbackIds)
    {
        layerConfig.RemoveChangeNotify(id);
    }
    dataCallbackIds.clear();
    for (const auto &element : data2)
    {
        CHECK_VALUE(int, dataCallbackIds);
        CHECK_VALUE(unsigned int, dataCallbackIds);
        CHECK_VALUE(long long, dataCallbackIds);
        CHECK_VALUE(unsigned long long, dataCallbackIds);
        CHECK_VALUE(bool, dataCallbackIds);
        CHECK_VALUE(double, dataCallbackIds);
        CHECK_VALUE(std::string, dataCallbackIds);
    }
    SetConfig(data2, config2View);
    EXPECT_EQ(notifyCount, data1.size() * 2);
    SetConfig(data1, config1View);
    EXPECT_EQ(notifyCount, data1.size() * 2); // config1 优先级低

    CheckConfig(data1, *config1);
    CheckConfig(data2, *config2);
    for (const auto &id : dataCallbackIds)
    {
        layerConfig.RemoveChangeNotify(id);
    }
    dataCallbackIds.clear();
    for (const auto &element : data1)
    {
        CHECK_VALUE(int, dataCallbackIds);
        CHECK_VALUE(unsigned int, dataCallbackIds);
        CHECK_VALUE(long long, dataCallbackIds);
        CHECK_VALUE(unsigned long long, dataCallbackIds);
        CHECK_VALUE(bool, dataCallbackIds);
        CHECK_VALUE(double, dataCallbackIds);
        CHECK_VALUE(std::string, dataCallbackIds);
    }
    SetConfig(data1, layerView);
    EXPECT_EQ(notifyCount, data1.size() * 3);

    CheckConfig(data1, layerView);
    CheckConfig(data1, *config2);
}

TEST(Config, Serialization)
{
    const std::string kAESKey  = "9c38fd2138ebda58da2a43ea008d86a9";
    const std::string kAESIv   = "0c86747b7460c619";
    const auto        filePath = std::filesystem::temp_directory_path() / "test.json";
    {
        auto          serializer = std::make_shared<FileSerializer>(filePath);
        GeneralConfig config(serializer);
        ConfigView    view(config);
        ConfigRandomTest(
            view, [&config](

                  ) { ASSERT_TRUE(config.Load()); }
        );
    }
    {
        auto serializer = std::make_shared<FileSerializer>(filePath);
        serializer->SetDigest(std::make_shared<Md5Digest>());
        GeneralConfig config(serializer);
        ConfigView    view(config);
        ConfigRandomTest(
            view, [&config](

                  ) { ASSERT_TRUE(config.Load()); }
        );
    }
    {
        auto serializer = std::make_shared<FileSerializer>(filePath);
        serializer->SetDigest(std::make_shared<Md5Digest>());
        serializer->SetCrypt(
            std::make_shared<AesEncrypt>(AESMode::CFB, AESPadding::PKCS7, kAESKey, kAESIv),
            std::make_shared<AesDecrypt>(AESMode::CFB, AESPadding::PKCS7, kAESKey, kAESIv)
        );
        GeneralConfig config(serializer);
        ConfigView    view(config);
        ConfigRandomTest(
            view, [&config](

                  ) { ASSERT_TRUE(config.Load()); }
        );
    }
    {
        auto          serializer = std::make_shared<FileSerializer>(filePath);
        GeneralConfig config(serializer);
        ConfigView    view(config);
        ConfigRandomTest(
            view,
            [&config, filePath](

            )
            {
                std::ifstream file(filePath, std::ios::binary);
                ASSERT_TRUE(file);
                std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                config.SetConfigMap(nlohmann::json::parse(data));
            }
        );
    }

    {
        auto serializer = std::make_shared<FileSerializer>(filePath);

        serializer->SetDigest(std::make_shared<Md5Digest>());
        GeneralConfig config(serializer);
        ConfigView    view(config);
        ConfigRandomTest(
            view,
            [&config, filePath](

            )
            {
                auto          digest = std::make_shared<Md5Digest>();
                std::ifstream file(filePath, std::ios::binary);
                ASSERT_TRUE(file);
                std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                EXPECT_EQ(digest->Name(), data.substr(0, digest->Name().size()));
                auto content = data.substr(digest->Name().size() + digest->GetSize());
                auto md5     = data.substr(digest->Name().size(), digest->GetSize());
                digest->Update(content);
                EXPECT_EQ(md5, std::string(reinterpret_cast<const char *>(digest->Digest()), digest->GetSize()));
                config.SetConfigMap(nlohmann::json::parse(content));
            }
        );
    }
}

TEST(Config, Null)
{
    // a key should be treated as not exist if its value is null
    auto key = "a.b.c";

    GeneralConfig config;
    ConfigView    view(config);
    view.Set(key, nullptr);

    EXPECT_FALSE(config.HasConfigValue(key));
    ASSERT_FALSE(config.GetConfigValue(key).has_value());
    EXPECT_TRUE(config.GetConfigValue(key).error() == ConfigError::kNotFound);
    EXPECT_FALSE(config.GetConfigKeys("a").empty());
    EXPECT_TRUE(config.GetConfigKeys("a").front() == "b");
    EXPECT_TRUE(config.GetConfigKeys("a.b").empty());

    EXPECT_FALSE(view.Has(key));
    ASSERT_FALSE(view.Get<std::nullptr_t>(key).has_value());
    EXPECT_TRUE(view.Get<std::nullptr_t>(key).error() == ConfigError::kNotFound);
    ASSERT_FALSE(view.Get<int>(key).has_value());
    EXPECT_TRUE(view.Get<int>(key).error() == ConfigError::kNotFound);
    EXPECT_FALSE(view.Keys("a").empty());
    EXPECT_TRUE(view.Keys("a").front() == "b");
    EXPECT_TRUE(view.Keys("a.b").empty());
}

TEST(Config, RootManipulation)
{
    // get/set/remove the root key
    constexpr auto rootKey = "";

    ConfigMap const data = nlohmann::json::parse(R"""({"key": "val"})""");

    GeneralConfig config;
    ConfigView    view(config);

    EXPECT_FALSE(view.Has(rootKey));

    // set and get root
    view.Set(rootKey, data);
    EXPECT_TRUE(view.Has(rootKey));
    auto const root = view.Get<ConfigMap>(rootKey).value();
    EXPECT_TRUE(root == data);

    // set and get
    view.Set(rootKey, data);
    EXPECT_TRUE(view.Has("key"));
    EXPECT_TRUE(view.Get<std::string>("key").value() == "val");

    // set null
    view.Set(rootKey, nullptr);
    EXPECT_FALSE(view.Has(rootKey));
    EXPECT_FALSE(view.Has("key"));

    // remove
    view.Set(rootKey, data);
    view.Remove(rootKey);
    EXPECT_FALSE(view.Has(rootKey));
    EXPECT_FALSE(view.Has("key"));
}

TEST(Config, EmptyKey)
{
    constexpr auto emptyKey  = "/";
    constexpr auto emptyKey2 = "a/";

    ConfigMap const data = nlohmann::json::parse(R"""(
{
    "": "val",
    "a": {
        "": "val"
    }
}
)""");

    GeneralConfig config;
    ConfigView    view(config);

    EXPECT_FALSE(view.Has(emptyKey));

    // set and get root
    view.Set("", data);
    EXPECT_TRUE(view.Has(emptyKey));
    EXPECT_TRUE(view.Get<ConfigMap>(emptyKey).value() == "val");
    EXPECT_TRUE(view.Has(emptyKey2));
    EXPECT_TRUE(view.Get<ConfigMap>(emptyKey2).value() == "val");

    // set null
    view.Set(emptyKey, nullptr);
    EXPECT_FALSE(view.Has(emptyKey));
    view.Set(emptyKey2, nullptr);
    EXPECT_FALSE(view.Has(emptyKey2));

    // remove
    view.Set(emptyKey, data);
    view.Remove(emptyKey);
    EXPECT_FALSE(view.Has(emptyKey));
    view.Set(emptyKey2, data);
    view.Remove(emptyKey2);
    EXPECT_FALSE(view.Has(emptyKey2));
}

TEST(Config, ViewPrefix)
{
    ConfigMap const data = nlohmann::json::parse(R"""(
{
    "": "val",
    "a": {
        "": "val"
    }
}
)""");

    GeneralConfig config;
    ConfigView    view_root(config);
    ConfigView    view_emptykey(config, "/");
    ConfigView    view_a(config, "a");
    ConfigView    view_a_emptykey(config, "a/");

    // has value when root is null
    EXPECT_FALSE(view_root.Has(""));
    EXPECT_FALSE(view_emptykey.Has(""));
    EXPECT_FALSE(view_a.Has(""));
    EXPECT_FALSE(view_a_emptykey.Has(""));

    // set and get
    view_root.Set("", data);
    EXPECT_TRUE(view_root.Has(""));
    EXPECT_TRUE(view_emptykey.Has(""));
    EXPECT_TRUE(view_a.Has(""));
    EXPECT_TRUE(view_a_emptykey.Has(""));
    EXPECT_TRUE(view_root.Get<ConfigMap>("").value() == data);
    EXPECT_TRUE(view_emptykey.Get<ConfigMap>("").value() == "val");
    EXPECT_TRUE(view_a.Get<ConfigMap>("").value() == nlohmann::json::parse(R"({"": "val"})"));
    EXPECT_TRUE(view_a.Get<ConfigMap>("/").value() == "val");
    EXPECT_TRUE(view_a_emptykey.Get<ConfigMap>("").value() == "val");

    // change child value by parent
    view_root.Set("", data);
    view_a.Set("/", "val2");
    EXPECT_TRUE(view_a_emptykey.Get<std::string>("").value() == "val2");
}
