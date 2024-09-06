#include <gtest/gtest.h>
#include <map>
#include <array>
#include <set>
#include <unordered_map>
#include <iterator>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/thread/thread_pool.h>
#include <zeus/foundation/sync/latch.h>
#include <zeus/foundation/container/concurrent_list.hpp>
#include <zeus/foundation/container/concurrent_vector.hpp>
#include <zeus/foundation/container/concurrent_queue.hpp>
#include <zeus/foundation/container/concurrent_map.hpp>
#include <zeus/foundation/container/concurrent_unordered_map.hpp>
#include <zeus/foundation/container/concurrent_multimap.hpp>
#include <zeus/foundation/container/concurrent_unordered_multimap.hpp>
#include <zeus/foundation/container/container_cast.hpp>
#include <zeus/foundation/container/callback_manager.hpp>
#include <zeus/foundation/container/cache_manager.hpp>
#include <zeus/foundation/container/filter_manager.hpp>
#include <zeus/foundation/container/fixed_buffer_queue.hpp>
#include <zeus/foundation/time/time.h>
#include "move_test.hpp"
using namespace std;
using namespace zeus;

namespace
{
const char* TEST1_DATA = "11dadasdsdadass";
const char* TEST2_DATA = "22222dadasdsdadass";
const char* TEST3_DATA = "33333333dadasdsdadass";
const char* TEST4_DATA = "444444444444dadasdsdadass";
}

template<typename Type>
void CheckValueContainer(const Type& container, size_t size, size_t valueOffset = 0)
{
    std::vector<bool> flag(size);
    for (const auto& item : container)
    {
        flag[item - valueOffset] = true;
    }
    EXPECT_TRUE(std::all_of(flag.begin(), flag.end(), [](const bool& item) { return item; }));
}

template<typename Type>
void CheckKeyCount(const Type& container, size_t size, size_t valueOffset = 0)
{
    std::vector<bool> flag(size);
    for (const auto& item : container)
    {
        flag[item - valueOffset] = true;
    }
    EXPECT_TRUE(std::all_of(flag.begin(), flag.end(), [](const bool& item) { return item; }));
}

template<typename Type>
void CheckMapContainer(const Type& container, size_t size, size_t valueOffset = 0)
{
    EXPECT_EQ(size, container.size());
    for (auto iter = container.begin(); iter != container.end(); ++iter)
    {
        EXPECT_EQ(iter->second, iter->first + valueOffset);
    }
}

TEST(Container, cast)
{
    {
        std::vector<int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace_back(i);
        }
        CheckValueContainer<decltype(data)>(data, 100);
        auto listData = VectorToList(data);
        CheckValueContainer<decltype(listData)>(listData, 100);
        auto keyVectorData = ListToVector(listData);
        CheckValueContainer(keyVectorData, 100);
    }
    {
        std::set<int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i);
        }
        CheckValueContainer<decltype(data)>(data, 100);
        auto keyVectorData = SetToVector(data);
        CheckValueContainer(keyVectorData, 100);
        auto setData = VectorToSet(keyVectorData);
        CheckValueContainer<decltype(setData)>(setData, 100);
    }
    {
        std::set<int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i);
        }
        CheckValueContainer<decltype(data)>(data, 100);
        auto listData = SetToList(data);
        CheckValueContainer<decltype(listData)>(listData, 100);
        auto setData = ListToSet(listData);
        CheckValueContainer<decltype(setData)>(setData, 100);
    }
    {
        const size_t       valueOffset = 3;
        std::map<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i + valueOffset);
        }
        CheckMapContainer(data, 100, valueOffset);
        auto keyVectorData = MapKeysToVector(data);
        CheckKeyCount(keyVectorData, 100);
        auto keyListData = MapKeysToList(data);
        CheckKeyCount(keyListData, 100);
        auto keySetData = MapKeysToSet(data);
        CheckKeyCount(keySetData, 100);
        auto keyUnorderedSetData = MapKeysToUnorderedSet(data);
        CheckKeyCount(keyUnorderedSetData, 100);
        auto valueVectorData = MapValuesToVector(data);
        CheckValueContainer(valueVectorData, 100, valueOffset);
        auto valueListData = MapValuesToList(data);
        CheckValueContainer(valueListData, 100, valueOffset);
    }

    {
        const size_t                 valueOffset = 3;
        std::unordered_map<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i + valueOffset);
        }
        CheckMapContainer(data, 100, valueOffset);
        auto keyVectorData = MapKeysToVector(data);
        CheckKeyCount(keyVectorData, 100);
        auto keyListData = MapKeysToList(data);
        CheckKeyCount(keyListData, 100);
        auto keySetData = MapKeysToSet(data);
        CheckKeyCount(keySetData, 100);
        auto keyUnorderedSetData = MapKeysToUnorderedSet(data);
        CheckKeyCount(keyUnorderedSetData, 100);
        auto valueVectorData = MapValuesToVector(data);
        CheckValueContainer(valueVectorData, 100, valueOffset);
        auto valueListData = MapValuesToList(data);
        CheckValueContainer(valueListData, 100, valueOffset);
    }
    {
        const size_t            valueOffset = 3;
        std::multimap<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i + valueOffset);
        }
        CheckMapContainer(data, 100, valueOffset);
        auto keyVectorData = MapKeysToVector(data);
        CheckKeyCount(keyVectorData, 100);
        auto keyListData = MapKeysToList(data);
        CheckKeyCount(keyListData, 100);
        auto keySetData = MapKeysToSet(data);
        CheckKeyCount(keySetData, 100);
        auto keyUnorderedSetData = MapKeysToUnorderedSet(data);
        CheckKeyCount(keyUnorderedSetData, 100);
        auto valueVectorData = MapValuesToVector(data);
        CheckValueContainer(valueVectorData, 100, valueOffset);
        auto valueListData = MapValuesToList(data);
        CheckValueContainer(valueListData, 100, valueOffset);
    }
    {
        const size_t                      valueOffset = 3;
        std::unordered_multimap<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i + valueOffset);
        }
        CheckMapContainer(data, 100, valueOffset);
        auto keyVectorData = MapKeysToVector(data);
        CheckKeyCount(keyVectorData, 100);
        auto keyListData = MapKeysToList(data);
        CheckKeyCount(keyListData, 100);
        auto keySetData = MapKeysToSet(data);
        CheckKeyCount(keySetData, 100);
        auto keyUnorderedSetData = MapKeysToUnorderedSet(data);
        CheckKeyCount(keyUnorderedSetData, 100);
        auto valueVectorData = MapValuesToVector(data);
        CheckValueContainer(valueVectorData, 100, valueOffset);
        auto valueListData = MapValuesToList(data);
        CheckValueContainer(valueListData, 100, valueOffset);
    }
    {
        const size_t       valueOffset = 3;
        std::map<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i + valueOffset);
        }
        CheckMapContainer(data, 100, valueOffset);
        auto mapData = MapToMap(data);
        CheckMapContainer(mapData, 100, valueOffset);
        data = MapToMap(mapData);
        CheckMapContainer(data, 100, valueOffset);
    }

    {
        const size_t            valueOffset = 3;
        std::multimap<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i + valueOffset);
        }
        CheckMapContainer(data, 100, valueOffset);
        auto mapData = MapToMap(data);
        CheckMapContainer(mapData, 100, valueOffset);
        data = MapToMap(mapData);
        CheckMapContainer(data, 100, valueOffset);
    }
    {
        const size_t       valueOffset = 3;
        std::map<int, int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i, i);
        }
        CheckMapContainer(data, 100, 0);
        auto mapData = MapToMap<int, int, int, int>(
            data, [&valueOffset](int value) -> int { return value; }, [&valueOffset](int value) -> int { return value + valueOffset; }
        );
        CheckMapContainer(mapData, 100, valueOffset);
    }
    {
        const size_t  valueOffset = 3;
        std::set<int> data;
        for (auto i = 0; i < 100; ++i)
        {
            data.emplace(i);
        }
        CheckValueContainer<decltype(data)>(data, 100);
        auto setData = SetToSet<int, int>(data, [&valueOffset](int value) -> int { return value + valueOffset; });
        CheckValueContainer<decltype(setData)>(setData, 100, valueOffset);
    }
}

TEST(Container, vector)
{
    {
        ConcurrentVector<string> container;
        container.Reserve(100);
        EXPECT_TRUE(container.Empty());

        container.EmplaceBack(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.PushBack(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.PushBack(std::move(temp3));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST3_DATA);
        EXPECT_EQ(container.Values().back(), TEST3_DATA);
        EXPECT_EQ(container.Back(), TEST3_DATA);

        EXPECT_EQ(container.At(0), TEST1_DATA);
        EXPECT_EQ(container.At(1), TEST2_DATA);
        EXPECT_EQ(container.At(2), TEST3_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST2_DATA);
        EXPECT_EQ(container.Values().back(), TEST2_DATA);
        EXPECT_EQ(container.Back(), TEST2_DATA);

        EXPECT_EQ(container.At(0), TEST1_DATA);
        EXPECT_EQ(container.At(1), TEST2_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST1_DATA);
        EXPECT_EQ(container.Values().back(), TEST1_DATA);
        EXPECT_EQ(container.Back(), TEST1_DATA);

        EXPECT_EQ(container.At(0), TEST1_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());
    }

    {
        ConcurrentVector<string, true> container;
        container.Reserve(100);
        EXPECT_TRUE(container.Empty());

        container.EmplaceBack(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.PushBack(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.PushBack(std::move(temp3));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST3_DATA);
        EXPECT_EQ(*container.Values().back(), TEST3_DATA);
        EXPECT_EQ(*container.Back(), TEST3_DATA);

        EXPECT_EQ(*container.At(0), TEST1_DATA);
        EXPECT_EQ(*container.At(1), TEST2_DATA);
        EXPECT_EQ(*container.At(2), TEST3_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST2_DATA);
        EXPECT_EQ(*container.Values().back(), TEST2_DATA);
        EXPECT_EQ(*container.Back(), TEST2_DATA);

        EXPECT_EQ(*container.At(0), TEST1_DATA);
        EXPECT_EQ(*container.At(1), TEST2_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST1_DATA);
        EXPECT_EQ(*container.Values().back(), TEST1_DATA);
        EXPECT_EQ(*container.Back(), TEST1_DATA);

        EXPECT_EQ(*container.At(0), TEST1_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());

        container.PushBack(std::make_shared<std::string>(TEST4_DATA));
        EXPECT_EQ(*container.Data().front(), TEST4_DATA);
    }
    {
        MoveTest                   test;
        ConcurrentVector<MoveTest> container;
        container.PushBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front().MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front().MoveAssign());
    }
    {
        MoveTest                   test;
        ConcurrentVector<MoveTest> container;
        container.EmplaceBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front().MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front().MoveAssign());
    }
    {
        MoveTest                         test;
        ConcurrentVector<MoveTest, true> container;
        container.PushBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front()->MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front()->MoveAssign());
    }
    {
        MoveTest                         test;
        ConcurrentVector<MoveTest, true> container;
        container.EmplaceBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front()->MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front()->MoveAssign());
    }
}

TEST(Container, list)
{
    {
        ConcurrentList<string> container;

        EXPECT_TRUE(container.Empty());

        container.EmplaceBack(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.PushBack(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.PushBack(std::move(temp3));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST3_DATA);
        EXPECT_EQ(container.Values().back(), TEST3_DATA);
        EXPECT_EQ(container.Back(), TEST3_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST2_DATA);
        EXPECT_EQ(container.Values().back(), TEST2_DATA);
        EXPECT_EQ(container.Back(), TEST2_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST1_DATA);
        EXPECT_EQ(container.Values().back(), TEST1_DATA);
        EXPECT_EQ(container.Back(), TEST1_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());
    }
    {
        ConcurrentList<string> container;

        EXPECT_TRUE(container.Empty());
        std::string temp3(TEST3_DATA);
        container.PushFront(std::move(temp3));

        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.PushFront(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        container.EmplaceFront(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST3_DATA);
        EXPECT_EQ(container.Values().back(), TEST3_DATA);
        EXPECT_EQ(container.Back(), TEST3_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST2_DATA);
        EXPECT_EQ(container.Values().back(), TEST2_DATA);
        EXPECT_EQ(container.Back(), TEST2_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST1_DATA);
        EXPECT_EQ(container.Values().back(), TEST1_DATA);
        EXPECT_EQ(container.Back(), TEST1_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());
    }

    {
        ConcurrentList<string, true> container;

        EXPECT_TRUE(container.Empty());

        container.EmplaceBack(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.PushBack(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.PushBack(std::move(temp3));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST3_DATA);
        EXPECT_EQ(*container.Values().back(), TEST3_DATA);
        EXPECT_EQ(*container.Back(), TEST3_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST2_DATA);
        EXPECT_EQ(*container.Values().back(), TEST2_DATA);
        EXPECT_EQ(*container.Back(), TEST2_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST1_DATA);
        EXPECT_EQ(*container.Values().back(), TEST1_DATA);
        EXPECT_EQ(*container.Back(), TEST1_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());

        container.PushBack(std::make_shared<std::string>(TEST4_DATA));
        EXPECT_EQ(*container.Data().front(), TEST4_DATA);
    }
    {
        ConcurrentList<string, true> container;

        EXPECT_TRUE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.PushFront(std::move(temp3));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.PushFront(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        container.EmplaceFront(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST3_DATA);
        EXPECT_EQ(*container.Values().back(), TEST3_DATA);
        EXPECT_EQ(*container.Back(), TEST3_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST2_DATA);
        EXPECT_EQ(*container.Values().back(), TEST2_DATA);
        EXPECT_EQ(*container.Back(), TEST2_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST1_DATA);
        EXPECT_EQ(*container.Values().back(), TEST1_DATA);
        EXPECT_EQ(*container.Back(), TEST1_DATA);

        container.PopBack();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());

        container.PushFront(std::make_shared<std::string>(TEST4_DATA));
        EXPECT_EQ(*container.Data().front(), TEST4_DATA);
    }
    {
        MoveTest                 test;
        ConcurrentList<MoveTest> container;
        container.PushBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front().MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front().MoveAssign());
    }
    {
        MoveTest                 test;
        ConcurrentList<MoveTest> container;
        container.EmplaceBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front().MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front().MoveAssign());
    }
    {
        MoveTest                       test;
        ConcurrentList<MoveTest, true> container;
        container.PushBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front()->MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front()->MoveAssign());
    }
    {
        MoveTest                       test;
        ConcurrentList<MoveTest, true> container;
        container.EmplaceBack(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front()->MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front()->MoveAssign());
    }
}

TEST(Container, queue)
{
    {
        ConcurrentQueue<string> container;

        EXPECT_TRUE(container.Empty());

        container.Emplace(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.Push(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.Push(std::move(temp3));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(container.Data().front(), TEST1_DATA);
        EXPECT_EQ(container.Values().front(), TEST1_DATA);
        EXPECT_EQ(container.Front(), TEST1_DATA);

        EXPECT_EQ(container.Data().back(), TEST3_DATA);
        EXPECT_EQ(container.Values().back(), TEST3_DATA);
        EXPECT_EQ(container.Back(), TEST3_DATA);

        container.Pop();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().front(), TEST2_DATA);
        EXPECT_EQ(container.Values().front(), TEST2_DATA);
        EXPECT_EQ(container.Front(), TEST2_DATA);

        EXPECT_EQ(container.Data().back(), TEST3_DATA);
        EXPECT_EQ(container.Values().back(), TEST3_DATA);
        EXPECT_EQ(container.Back(), TEST3_DATA);

        container.Pop();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(container.Data().back(), TEST3_DATA);
        EXPECT_EQ(container.Values().back(), TEST3_DATA);
        EXPECT_EQ(container.Back(), TEST3_DATA);

        EXPECT_EQ(container.Data().front(), TEST3_DATA);
        EXPECT_EQ(container.Values().front(), TEST3_DATA);
        EXPECT_EQ(container.Front(), TEST3_DATA);

        container.Pop();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());
    }

    {
        ConcurrentQueue<string, true> container;

        EXPECT_TRUE(container.Empty());

        container.Emplace(TEST1_DATA, strlen(TEST1_DATA));
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        std::string temp2(TEST2_DATA);
        container.Push(temp2);
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        std::string temp3(TEST3_DATA);
        container.Push(std::move(temp3));
        EXPECT_EQ(container.Size(), 3);
        EXPECT_FALSE(container.Empty());

        EXPECT_FALSE(temp2.empty());
        EXPECT_TRUE(temp3.empty());

        EXPECT_EQ(*container.Data().front(), TEST1_DATA);
        EXPECT_EQ(*container.Values().front(), TEST1_DATA);
        EXPECT_EQ(*container.Front(), TEST1_DATA);

        EXPECT_EQ(*container.Data().back(), TEST3_DATA);
        EXPECT_EQ(*container.Values().back(), TEST3_DATA);
        EXPECT_EQ(*container.Back(), TEST3_DATA);

        container.Pop();
        EXPECT_EQ(container.Size(), 2);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST2_DATA);
        EXPECT_EQ(*container.Values().front(), TEST2_DATA);
        EXPECT_EQ(*container.Front(), TEST2_DATA);

        EXPECT_EQ(*container.Data().back(), TEST3_DATA);
        EXPECT_EQ(*container.Values().back(), TEST3_DATA);
        EXPECT_EQ(*container.Back(), TEST3_DATA);

        container.Pop();
        EXPECT_EQ(container.Size(), 1);
        EXPECT_FALSE(container.Empty());

        EXPECT_EQ(*container.Data().front(), TEST3_DATA);
        EXPECT_EQ(*container.Values().front(), TEST3_DATA);
        EXPECT_EQ(*container.Front(), TEST3_DATA);

        EXPECT_EQ(*container.Data().back(), TEST3_DATA);
        EXPECT_EQ(*container.Values().back(), TEST3_DATA);
        EXPECT_EQ(*container.Back(), TEST3_DATA);

        container.Pop();
        EXPECT_EQ(container.Size(), 0);
        EXPECT_TRUE(container.Empty());

        container.Push(std::make_shared<std::string>(TEST4_DATA));
        EXPECT_EQ(*container.Data().front(), TEST4_DATA);
    }
    {
        MoveTest                  test;
        ConcurrentQueue<MoveTest> container;
        container.Push(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front().MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front().MoveAssign());
    }
    {
        MoveTest                  test;
        ConcurrentQueue<MoveTest> container;
        container.Emplace(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front().MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front().MoveAssign());
    }
    {
        MoveTest                        test;
        ConcurrentQueue<MoveTest, true> container;
        container.Push(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front()->MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front()->MoveAssign());
    }
    {
        MoveTest                        test;
        ConcurrentQueue<MoveTest, true> container;
        container.Emplace(std::move(test));
        EXPECT_TRUE(test.MoveConstruct());
        EXPECT_TRUE(container.Data().front()->MoveConstruct());
        EXPECT_FALSE(test.MoveAssign());
        EXPECT_FALSE(container.Data().front()->MoveAssign());
    }
}

TEST(Container, maptainer)
{
    {
        std::string                   temp(TEST2_DATA);
        ConcurrentMap<string, string> container;
        EXPECT_TRUE(container.Empty());

        EXPECT_TRUE(container.Set(TEST1_DATA, temp, true));
        EXPECT_EQ(TEST2_DATA, container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_FALSE(container.Set(TEST1_DATA, TEST3_DATA, false));
        EXPECT_EQ(temp, container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_EQ(temp, container.Values().front());
        EXPECT_EQ(TEST1_DATA, container.Keys().front());
        EXPECT_EQ(temp, container.Data()[TEST1_DATA]);

        EXPECT_TRUE(container.Set(TEST2_DATA, TEST4_DATA, false));
        EXPECT_EQ(TEST4_DATA, container.Get(TEST2_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_TRUE(container.Set(TEST1_DATA, TEST3_DATA, true));
        EXPECT_EQ(TEST3_DATA, container.Get(TEST1_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_EQ(TEST3_DATA, container.Data()[TEST1_DATA]);
        EXPECT_EQ(TEST4_DATA, container.Data()[TEST2_DATA]);
    }
    {
        std::string                         temp(TEST2_DATA);
        ConcurrentMap<string, string, true> container;
        EXPECT_TRUE(container.Empty());

        EXPECT_TRUE(container.Set(TEST1_DATA, temp, true));
        EXPECT_EQ(TEST2_DATA, *container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_FALSE(container.Set(TEST1_DATA, TEST3_DATA, false));
        EXPECT_EQ(temp, *container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_EQ(temp, *container.Values().front());
        EXPECT_EQ(TEST1_DATA, container.Keys().front());
        EXPECT_EQ(temp, *container.Data()[TEST1_DATA]);

        EXPECT_TRUE(container.Set(TEST2_DATA, TEST4_DATA, false));
        EXPECT_EQ(TEST4_DATA, *container.Get(TEST2_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_TRUE(container.Set(TEST1_DATA, TEST3_DATA, true));
        EXPECT_EQ(TEST3_DATA, *container.Get(TEST1_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_EQ(TEST3_DATA, *container.Data()[TEST1_DATA]);
        EXPECT_EQ(TEST4_DATA, *container.Data()[TEST2_DATA]);

        EXPECT_TRUE(container.Set(TEST4_DATA, std::make_shared<std::string>(TEST3_DATA), true));
        EXPECT_EQ(TEST3_DATA, *container.Get(TEST4_DATA));
        EXPECT_EQ(3, container.Size());
    }
    {
        MoveTest                        test;
        ConcurrentMap<string, MoveTest> container;
        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front().Moved());

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_FALSE(container.Set(TEST1_DATA, std::move(test), false));

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front().Moved());
    }
    {
        MoveTest                              test;
        ConcurrentMap<string, MoveTest, true> container;
        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front()->Moved());

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_FALSE(container.Set(TEST1_DATA, std::move(test), false));

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front()->Moved());
    }
}

TEST(Container, unordered_map)
{
    {
        std::string                            temp(TEST2_DATA);
        ConcurrentUnorderedMap<string, string> container;
        EXPECT_TRUE(container.Empty());

        EXPECT_TRUE(container.Set(TEST1_DATA, temp, true));
        EXPECT_EQ(TEST2_DATA, container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_FALSE(container.Set(TEST1_DATA, TEST3_DATA, false));
        EXPECT_EQ(temp, container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_EQ(temp, container.Values().front());
        EXPECT_EQ(TEST1_DATA, container.Keys().front());
        EXPECT_EQ(temp, container.Data()[TEST1_DATA]);

        EXPECT_TRUE(container.Set(TEST2_DATA, TEST4_DATA, false));
        EXPECT_EQ(TEST4_DATA, container.Get(TEST2_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_TRUE(container.Set(TEST1_DATA, TEST3_DATA, true));
        EXPECT_EQ(TEST3_DATA, container.Get(TEST1_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_EQ(TEST3_DATA, container.Data()[TEST1_DATA]);
        EXPECT_EQ(TEST4_DATA, container.Data()[TEST2_DATA]);
    }
    {
        std::string                                  temp(TEST2_DATA);
        ConcurrentUnorderedMap<string, string, true> container;
        EXPECT_TRUE(container.Empty());

        EXPECT_TRUE(container.Set(TEST1_DATA, temp, true));
        EXPECT_EQ(TEST2_DATA, *container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_FALSE(container.Set(TEST1_DATA, TEST3_DATA, false));
        EXPECT_EQ(temp, *container.Get(TEST1_DATA));
        EXPECT_EQ(1, container.Size());

        EXPECT_EQ(temp, *container.Values().front());
        EXPECT_EQ(TEST1_DATA, container.Keys().front());
        EXPECT_EQ(temp, *container.Data()[TEST1_DATA]);

        EXPECT_TRUE(container.Set(TEST2_DATA, TEST4_DATA, false));
        EXPECT_EQ(TEST4_DATA, *container.Get(TEST2_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_TRUE(container.Set(TEST1_DATA, TEST3_DATA, true));
        EXPECT_EQ(TEST3_DATA, *container.Get(TEST1_DATA));
        EXPECT_EQ(2, container.Size());

        EXPECT_EQ(TEST3_DATA, *container.Data()[TEST1_DATA]);
        EXPECT_EQ(TEST4_DATA, *container.Data()[TEST2_DATA]);

        EXPECT_TRUE(container.Set(TEST4_DATA, std::make_shared<std::string>(TEST3_DATA), true));
        EXPECT_EQ(TEST3_DATA, *container.Get(TEST4_DATA));
        EXPECT_EQ(3, container.Size());
    }
    {
        MoveTest                                 test;
        ConcurrentUnorderedMap<string, MoveTest> container;
        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front().Moved());

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_FALSE(container.Set(TEST1_DATA, std::move(test), false));

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front().Moved());
    }
    {
        MoveTest                                       test;
        ConcurrentUnorderedMap<string, MoveTest, true> container;
        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front()->Moved());

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_FALSE(container.Set(TEST1_DATA, std::move(test), false));

        test.Reset();
        EXPECT_FALSE(test.Moved());

        EXPECT_TRUE(container.Set(TEST1_DATA, std::move(test), true));
        EXPECT_TRUE(test.Moved());
        EXPECT_TRUE(container.Values().front()->Moved());
    }
}

TEST(Container, multimap)
{
    {
        ConcurrentMultiMap<string, string> container;
        EXPECT_EQ(0, container.Size());
        EXPECT_TRUE(container.Empty());
        std::multimap<std::string, std::string> testData = {
            {TEST1_DATA, TEST2_DATA},
            {TEST1_DATA, TEST3_DATA},
            {TEST2_DATA, TEST1_DATA},
            {TEST3_DATA, TEST4_DATA}
        };

        for (const auto& item : testData)
        {
            container.Set(item.first, item.second);
        }
        EXPECT_EQ(testData.size(), container.Size());
        EXPECT_FALSE(container.Empty());
        for (const auto& item : testData)
        {
            EXPECT_TRUE(container.Data().find(item.first) != container.Data().end());
            std::set<std::string> data  = VectorToSet(container.Get(item.first));
            auto                  range = testData.equal_range(item.first);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                EXPECT_EQ(1, data.count(iter->second));
            }
        }
        {
            std::set<std::string> keys = VectorToSet(container.Keys());
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, keys.count(item.first));
            }
        }
        {
            std::set<std::string> values = VectorToSet(container.Values());
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, values.count(item.second));
            }
        }
    }
    {
        ConcurrentMultiMap<string, string, true> container;
        EXPECT_EQ(0, container.Size());
        EXPECT_TRUE(container.Empty());
        std::multimap<std::string, std::string> testData = {
            {TEST1_DATA, TEST2_DATA},
            {TEST1_DATA, TEST3_DATA},
            {TEST2_DATA, TEST1_DATA},
            {TEST3_DATA, TEST4_DATA}
        };

        for (const auto& item : testData)
        {
            container.Set(item.first, item.second);
        }
        EXPECT_EQ(testData.size(), container.Size());
        EXPECT_FALSE(container.Empty());
        for (const auto& item : testData)
        {
            EXPECT_TRUE(container.Data().find(item.first) != container.Data().end());
            std::set<std::shared_ptr<std::string>> ptrData = VectorToSet(container.Get(item.first));
            std::set<std::string>                  data;
            std::transform(
                ptrData.begin(), ptrData.end(), std::insert_iterator<decltype(data)>(data, data.begin()),
                [](const std::shared_ptr<std::string>& data) { return *data; }
            );

            auto range = testData.equal_range(item.first);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                EXPECT_EQ(1, data.count((iter)->second));
            }
        }
        {
            std::set<std::string> keys = VectorToSet(container.Keys());
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, keys.count(item.first));
            }
        }
        {
            std::set<std::shared_ptr<std::string>> ptrValues = VectorToSet(container.Values());
            std::set<std::string>                  values;
            for (const auto& item : ptrValues)
            {
                values.emplace(*item);
            }
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, values.count(item.second));
            }
        }
    }
}

TEST(Container, unordered_multimap)
{
    {
        ConcurrentUnorderedMultiMap<string, string> container;
        EXPECT_EQ(0, container.Size());
        EXPECT_TRUE(container.Empty());
        std::multimap<std::string, std::string> testData = {
            {TEST1_DATA, TEST2_DATA},
            {TEST1_DATA, TEST3_DATA},
            {TEST2_DATA, TEST1_DATA},
            {TEST3_DATA, TEST4_DATA}
        };

        for (const auto& item : testData)
        {
            container.Set(item.first, item.second);
        }
        EXPECT_EQ(testData.size(), container.Size());
        EXPECT_FALSE(container.Empty());
        for (const auto& item : testData)
        {
            EXPECT_TRUE(container.Data().find(item.first) != container.Data().end());
            std::set<std::string> data  = VectorToSet(container.Get(item.first));
            auto                  range = testData.equal_range(item.first);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                EXPECT_EQ(1, data.count(iter->second));
            }
        }
        {
            std::set<std::string> keys = VectorToSet(container.Keys());
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, keys.count(item.first));
            }
        }
        {
            std::set<std::string> values = VectorToSet(container.Values());
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, values.count(item.second));
            }
        }
    }
    {
        ConcurrentUnorderedMultiMap<string, string, true> container;
        EXPECT_EQ(0, container.Size());
        EXPECT_TRUE(container.Empty());
        std::multimap<std::string, std::string> testData = {
            {TEST1_DATA, TEST2_DATA},
            {TEST1_DATA, TEST3_DATA},
            {TEST2_DATA, TEST1_DATA},
            {TEST3_DATA, TEST4_DATA}
        };

        for (const auto& item : testData)
        {
            container.Set(item.first, item.second);
        }
        EXPECT_EQ(testData.size(), container.Size());
        EXPECT_FALSE(container.Empty());
        for (const auto& item : testData)
        {
            EXPECT_TRUE(container.Data().find(item.first) != container.Data().end());
            std::set<std::shared_ptr<std::string>> ptrData = VectorToSet(container.Get(item.first));
            std::set<std::string>                  data;
            std::transform(
                ptrData.begin(), ptrData.end(), std::insert_iterator<decltype(data)>(data, data.begin()),
                [](const std::shared_ptr<std::string>& data) { return *data; }
            );
            auto range = testData.equal_range(item.first);
            for (auto iter = range.first; iter != range.second; ++iter)
            {
                EXPECT_EQ(1, data.count((iter)->second));
            }
        }
        {
            std::set<std::string> keys = VectorToSet(container.Keys());
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, keys.count(item.first));
            }
        }
        {
            std::set<std::shared_ptr<std::string>> ptrValues = VectorToSet(container.Values());
            std::set<std::string>                  values;
            for (const auto& item : ptrValues)
            {
                values.emplace(*item);
            }
            for (const auto& item : testData)
            {
                EXPECT_EQ(1, values.count(item.second));
            }
        }
    }
}

TEST(CallbackManager, limit)
{
    {
        CallbackManager<size_t> manager(5);
        EXPECT_TRUE(manager.AddCallback([](size_t) {}));
        EXPECT_TRUE(manager.AddCallback([](size_t) {}));
        EXPECT_TRUE(manager.AddCallback([](size_t) {}));
        EXPECT_TRUE(manager.AddCallback([](size_t) {}));
        EXPECT_TRUE(manager.AddCallback([](size_t) {}));
        EXPECT_FALSE(manager.AddCallback([](size_t) {}));
        EXPECT_FALSE(manager.AddCallback([](size_t) {}));
    }
    {
        CallbackManager<> manager(5);
        EXPECT_TRUE(manager.AddCallback([]() {}));
        EXPECT_TRUE(manager.AddCallback([]() {}));
        EXPECT_TRUE(manager.AddCallback([]() {}));
        EXPECT_TRUE(manager.AddCallback([]() {}));
        EXPECT_TRUE(manager.AddCallback([]() {}));
        EXPECT_FALSE(manager.AddCallback([]() {}));
        EXPECT_FALSE(manager.AddCallback([]() {}));
    }
    {
        NameCallbackManager<std::string, size_t> manager(5);
        EXPECT_TRUE(manager.AddCallback("call", [](size_t) {}));
        EXPECT_TRUE(manager.AddCallback("call", [](size_t) {}));
        EXPECT_TRUE(manager.AddCallback("call", [](size_t) {}));
        EXPECT_TRUE(manager.AddCallback("call", [](size_t) {}));
        EXPECT_TRUE(manager.AddCallback("call", [](size_t) {}));
        EXPECT_FALSE(manager.AddCallback("call", [](size_t) {}));
        EXPECT_FALSE(manager.AddCallback("call", [](size_t) {}));
    }
    {
        NameCallbackManager<std::string> manager(5);
        EXPECT_TRUE(manager.AddCallback("call", []() {}));
        EXPECT_TRUE(manager.AddCallback("call", []() {}));
        EXPECT_TRUE(manager.AddCallback("call", []() {}));
        EXPECT_TRUE(manager.AddCallback("call", []() {}));
        EXPECT_TRUE(manager.AddCallback("call", []() {}));
        EXPECT_FALSE(manager.AddCallback("call", []() {}));
        EXPECT_FALSE(manager.AddCallback("call", []() {}));
    }
}

TEST(CallbackManager, WaitRemove)
{
    CallbackManager<size_t>      manager;
    std::shared_ptr<std::string> count = std::make_shared<std::string>();
    EXPECT_EQ(1, count.use_count());
    size_t callbackId = manager.AddCallback([count, &manager](size_t callbackId) { manager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());

    manager.Call(callbackId);

    EXPECT_EQ(1, count.use_count());
    callbackId = manager.AddCallback([count, &manager](size_t callbackId) { manager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());
    manager.Call(callbackId);
    EXPECT_EQ(1, count.use_count());
    NameCallbackManager<std::string, size_t> nameManager;
    callbackId = nameManager.AddCallback("call", [count, &nameManager](size_t callbackId) { nameManager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());

    nameManager.Call("call", callbackId);

    EXPECT_EQ(1, count.use_count());
    callbackId = nameManager.AddCallback("call", [count, &nameManager](size_t callbackId) { nameManager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());
    nameManager.Call("call", callbackId);
    EXPECT_EQ(1, count.use_count());
}

TEST(CallbackManager, WaitRemoveVoid)
{
    CallbackManager<>            manager;
    std::shared_ptr<std::string> count = std::make_shared<std::string>();
    EXPECT_EQ(1, count.use_count());
    size_t callbackId = manager.AddCallback([count, &manager, &callbackId]() { manager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());

    manager.Call();

    EXPECT_EQ(1, count.use_count());
    callbackId = manager.AddCallback([count, &manager, &callbackId]() { manager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());
    manager.Call();
    EXPECT_EQ(1, count.use_count());
    NameCallbackManager<std::string> nameManager;
    callbackId = nameManager.AddCallback("call", [count, &nameManager, &callbackId]() { nameManager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());

    nameManager.Call("call");

    EXPECT_EQ(1, count.use_count());
    callbackId = nameManager.AddCallback("call", [count, &nameManager, &callbackId]() { nameManager.RemoveCallback(callbackId, false); });
    EXPECT_EQ(2, count.use_count());
    nameManager.Call("call");
    EXPECT_EQ(1, count.use_count());
}

TEST(CallbackManager, Clear)
{
    CallbackManager<size_t> manager;
    std::set<size_t>        callbackIds;
    callbackIds.emplace(manager.AddCallback([&manager](size_t) {}));
    callbackIds.emplace(manager.AddCallback([&manager](size_t) {}));
    callbackIds.emplace(manager.AddCallback([&manager](size_t callbackId) { manager.RemoveCallback(callbackId); }));
    EXPECT_EQ(callbackIds.size(), 3);
    EXPECT_EQ(manager.CallbackIds(), callbackIds);
    EXPECT_EQ(manager.Size(), callbackIds.size());
    manager.Call(*callbackIds.begin());
    callbackIds.erase(callbackIds.begin());
    EXPECT_EQ(callbackIds.size(), 2);
    EXPECT_EQ(manager.CallbackIds(), callbackIds);
    EXPECT_EQ(manager.Size(), callbackIds.size());

    callbackIds.clear();
    NameCallbackManager<std::string, size_t> nameManager;
    callbackIds.emplace(nameManager.AddCallback("call", [&nameManager](size_t) {}));
    callbackIds.emplace(nameManager.AddCallback("call", [&nameManager](size_t) {}));
    callbackIds.emplace(nameManager.AddCallback("call", [&nameManager](size_t callbackId) { nameManager.RemoveCallback(callbackId); }));
    EXPECT_EQ(callbackIds.size(), 3);
    EXPECT_EQ(nameManager.CallbackIds(), callbackIds);
    EXPECT_EQ(nameManager.Size(), callbackIds.size());
    nameManager.Call("call", *callbackIds.begin());
    callbackIds.erase(callbackIds.begin());
    EXPECT_EQ(callbackIds.size(), 2);
    EXPECT_EQ(nameManager.CallbackIds(), callbackIds);
    EXPECT_EQ(nameManager.Size(), callbackIds.size());
}

TEST(CallbackManager, Clear1)
{
    CallbackManager<> manager;
    std::set<size_t>  callbackIds;
    callbackIds.emplace(manager.AddCallback([&manager]() {}));
    callbackIds.emplace(manager.AddCallback([&manager]() {}));
    callbackIds.emplace(manager.AddCallback([&manager, &callbackIds]() { manager.RemoveCallback(*callbackIds.begin()); }));
    EXPECT_EQ(callbackIds.size(), 3);
    EXPECT_EQ(manager.CallbackIds(), callbackIds);
    EXPECT_EQ(manager.Size(), callbackIds.size());
    manager.Call();
    callbackIds.erase(callbackIds.begin());
    EXPECT_EQ(callbackIds.size(), 2);
    EXPECT_EQ(manager.CallbackIds(), callbackIds);
    EXPECT_EQ(manager.Size(), callbackIds.size());

    callbackIds.clear();
    NameCallbackManager<std::string> nameManager;
    callbackIds.emplace(nameManager.AddCallback("call", [&nameManager]() {}));
    callbackIds.emplace(nameManager.AddCallback("call", [&nameManager]() {}));
    callbackIds.emplace(nameManager.AddCallback("call", [&nameManager, &callbackIds]() { nameManager.RemoveCallback(*callbackIds.begin()); }));
    EXPECT_EQ(callbackIds.size(), 3);
    EXPECT_EQ(nameManager.CallbackIds(), callbackIds);
    EXPECT_EQ(nameManager.Size(), callbackIds.size());
    nameManager.Call("call");
    callbackIds.erase(callbackIds.begin());
    EXPECT_EQ(callbackIds.size(), 2);
    EXPECT_EQ(nameManager.CallbackIds(), callbackIds);
    EXPECT_EQ(nameManager.Size(), callbackIds.size());
}

TEST(CallbackManager, ConcurrentRemove)
{
    zeus::ThreadPool                             pool(6);
    std::unordered_map<std::string, std::string> testData;
    for (auto i = 0; i < 100; i++)
    {
        testData[RandWord()] = RandWord();
    }
    std::unordered_map<std::string, atomic<size_t>> resultData;
    for (auto& iter : testData)
    {
        resultData[iter.first] = 0;
    }
    CallbackManager<const std::string&, const std::string&>               manager;
    std::function<void(const std::string& key, const std::string& value)> callback =
        [&testData, &resultData](const std::string& key, const std::string& value)
    {
        EXPECT_EQ(testData[key], value);
        resultData[key]++;
    };

    class Warp
    {
    public:
        Warp(const decltype(callback)& cc) : c(cc) {}
        ~Warp() { c = nullptr; }
        void operator()(const std::string& key, const std::string& value) { c(key, value); }

    private:
        decltype(callback) c;
    };

    EXPECT_EQ(resultData.size(), testData.size());
    EXPECT_FALSE(manager.Call("", ""));
    EXPECT_EQ(resultData.size(), testData.size());
    const size_t           callbackCount = 10000;
    ConcurrentList<size_t> ids;
    zeus::Latch            regLatch(callbackCount);
    for (size_t i = 0; i < callbackCount; i++)
    {
        pool.Commit(
            [&manager, &ids, &callback, &regLatch]()
            {
                auto id = manager.AddCallback(Warp(callback));
                ids.PushBack(id);
                EXPECT_FALSE(manager.Empty());
                regLatch.CountDown();
            }
        );
    }
    regLatch.Wait();
    EXPECT_EQ(callbackCount, manager.Size());

    zeus::Latch callLatch(testData.size());
    zeus::Latch removeLatch(ids.Size());
    for (auto& iter : testData)
    {
        pool.Commit(
            [&manager, &iter, &callLatch]()
            {
                manager.Call(iter.first, iter.second);
                callLatch.CountDown();
            }
        );
    }
    for (auto& id : ids.Data())
    {
        pool.Commit(
            [&manager, id, &removeLatch]()
            {
                EXPECT_TRUE(manager.RemoveCallback(id));
                removeLatch.CountDown();
            }
        );
    }

    EXPECT_EQ(resultData.size(), testData.size());

    callLatch.Wait();
    removeLatch.Wait();
    EXPECT_TRUE(manager.Empty());

    for (auto& iter : testData)
    {
        EXPECT_FALSE(manager.Call(iter.first, iter.second));
    }
    for (auto& id : ids.Data())
    {
        EXPECT_FALSE(manager.RemoveCallback(id));
    }
    EXPECT_EQ(resultData.size(), testData.size());
}

TEST(CallbackManager, Concurrent)
{
    zeus::ThreadPool                             pool(6);
    std::unordered_map<std::string, std::string> testData;
    for (auto i = 0; i < 100; i++)
    {
        testData[RandWord()] = RandWord();
    }
    std::unordered_map<std::string, atomic<size_t>> resultData;
    for (auto& iter : testData)
    {
        resultData[iter.first] = 0;
    }
    CallbackManager<const std::string&, const std::string&>               manager;
    std::atomic<bool>                                                     live(true);
    std::function<void(const std::string& key, const std::string& value)> callback =
        [&testData, &resultData, &live](const std::string& key, const std::string& value)
    {
        EXPECT_EQ(testData[key], value);
        resultData[key]++;
        EXPECT_TRUE(live);
    };

    EXPECT_EQ(resultData.size(), testData.size());
    EXPECT_FALSE(manager.Call("", ""));
    EXPECT_EQ(resultData.size(), testData.size());
    const size_t           callbackCount = 10000;
    ConcurrentList<size_t> ids;
    zeus::Latch            regLatch(callbackCount);
    for (size_t i = 0; i < callbackCount; i++)
    {
        pool.Commit(
            [&manager, &ids, &callback, &regLatch]()
            {
                auto id = manager.AddCallback(callback);
                ids.PushBack(id);
                EXPECT_FALSE(manager.Empty());
                regLatch.CountDown();
            }
        );
    }
    regLatch.Wait();
    EXPECT_EQ(callbackCount, manager.Size());

    zeus::Latch callLatch(testData.size());
    for (auto& iter : testData)
    {
        pool.Commit(
            [&manager, &iter, &callLatch]()
            {
                EXPECT_TRUE(manager.Call(iter.first, iter.second));
                callLatch.CountDown();
            }
        );
    }
    callLatch.Wait();
    EXPECT_EQ(resultData.size(), testData.size());
    for (auto& iter : resultData)
    {
        EXPECT_EQ(iter.second, callbackCount);
    }
    zeus::Latch removeLatch(ids.Size());
    for (auto& id : ids.Data())
    {
        pool.Commit(
            [&manager, id, &removeLatch]()
            {
                EXPECT_TRUE(manager.RemoveCallback(id));
                removeLatch.CountDown();
            }
        );
    }
    removeLatch.Wait();
    live = false;
    EXPECT_TRUE(manager.Empty());
    for (auto& iter : testData)
    {
        EXPECT_FALSE(manager.Call(iter.first, iter.second));
    }
    for (auto& id : ids.Data())
    {
        EXPECT_FALSE(manager.RemoveCallback(id));
    }
    EXPECT_EQ(resultData.size(), testData.size());
}

TEST(NameCallbackManager, Concurrent)
{
    zeus::ThreadPool                             pool(6);
    const size_t                                 nameCount = 200; //这里为了方便，固定为偶数
    std::unordered_map<std::string, std::string> testData;
    for (auto i = 0; i < 100; i++)
    {
        testData[RandWord()] = RandWord();
    }
    std::unordered_map<std::string, atomic<size_t>> resultData;
    for (auto& iter : testData)
    {
        resultData[iter.first] = 0;
    }
    std::unordered_map<std::string, atomic<size_t>> nameData;
    for (size_t i = 1; i <= nameCount; i++)
    {
        nameData[std::to_string(i)] = 0;
    }
    NameCallbackManager<std::string, const std::string&, const std::string&, const std::string&>   manager;
    std::atomic<bool>                                                                              live(true);
    std::function<void(const std::string& key, const std::string& value, const std::string& name)> callback =
        [&testData, &resultData, &nameData, &live](const std::string& key, const std::string& value, const std::string& name)
    {
        EXPECT_EQ(testData[key], value);
        resultData[key]++;
        nameData[name]++;
        EXPECT_TRUE(live);
    };

    EXPECT_EQ(resultData.size(), testData.size());
    EXPECT_FALSE(manager.Call("", "", "", ""));
    EXPECT_EQ(resultData.size(), testData.size());

    zeus::ConcurrentUnorderedMap<size_t, ConcurrentList<size_t>, true> nameIds;
    auto                                                               totalCallbackCount = (nameCount + 1) * (nameCount / 2);
    zeus::Latch regLatch(totalCallbackCount); //总共有1+2+……+nameCount个回调，等差数列
    for (size_t name = 1; name <= nameCount; name++)
    {
        auto ids = std::make_shared<ConcurrentList<size_t>>();
        nameIds.Set(name, ids);
        for (auto count = name; count; count--)
        {
            //根据名称的数字重复注册对应个数个回调
            pool.Commit(
                [&manager, name, &nameIds, &callback, &regLatch]()
                {
                    auto id = manager.AddCallback(std::to_string(name), callback);
                    nameIds.Get(name)->PushBack(id);
                    EXPECT_FALSE(manager.Empty());
                    EXPECT_FALSE(manager.Empty(std::to_string(name)));
                    regLatch.CountDown();
                }
            );
        }
    }
    regLatch.Wait();
    EXPECT_EQ(totalCallbackCount, manager.Size());
    for (size_t name = 1; name <= nameCount; name++)
    {
        EXPECT_FALSE(manager.Empty(std::to_string(name)));
        EXPECT_EQ(manager.Size(std::to_string(name)), name);
    }

    zeus::Latch callLatch(testData.size() * nameCount);
    for (auto& iter : testData)
    {
        for (size_t name = 1; name <= nameCount; name++)
        {
            pool.Commit(
                [&manager, &iter, &callLatch, name]()
                {
                    EXPECT_TRUE(manager.Call(std::to_string(name), iter.first, iter.second, std::to_string(name)));
                    callLatch.CountDown();
                }
            );
        }
    }
    callLatch.Wait();
    EXPECT_EQ(resultData.size(), testData.size());
    for (auto& iter : resultData)
    {
        EXPECT_EQ(iter.second, totalCallbackCount);
    }
    for (size_t i = 1; i <= nameCount; i++)
    {
        EXPECT_EQ(nameData[std::to_string(i)], i * testData.size());
    }
    zeus::Latch removeLatch(totalCallbackCount);
    auto        lastName = nameCount;
    auto        randName = RandUint32(1, nameCount - 1);
    EXPECT_TRUE(manager.RemoveCallback(std::to_string(lastName)));
    EXPECT_TRUE(manager.RemoveCallback(std::to_string(randName)));
    EXPECT_TRUE(manager.Empty(std::to_string(lastName)));
    EXPECT_TRUE(manager.Empty(std::to_string(randName)));
    for (auto& iter : nameIds.Data())
    {
        auto& name = iter.first;
        auto& ids  = iter.second;
        for (auto& id : ids->Data())
        {
            pool.Commit(
                [&manager, randName, lastName, name, id, &removeLatch]()
                {
                    if (name == lastName || name == randName)
                    {
                        EXPECT_FALSE(manager.RemoveCallback(id));
                    }
                    else
                    {
                        EXPECT_TRUE(manager.RemoveCallback(id));
                    }
                    removeLatch.CountDown();
                }
            );
        }
    }
    removeLatch.Wait();
    for (size_t name = 1; name <= nameCount; name++)
    {
        EXPECT_TRUE(manager.Empty(std::to_string(name)));
        EXPECT_EQ(manager.Size(std::to_string(name)), 0);
    }
    for (auto& iter : nameIds.Data())
    {
        auto& name = iter.first;
        EXPECT_FALSE(manager.RemoveCallback(name));
    }
    live = false;
    EXPECT_TRUE(manager.Empty());
    for (auto& iter : testData)
    {
        EXPECT_FALSE(manager.Call("", iter.first, iter.second, ""));
    }
    for (auto& iter : nameIds.Data())
    {
        for (auto& id : iter.second->Data())
        {
            EXPECT_FALSE(manager.RemoveCallback(id));
        }
    }
    EXPECT_EQ(resultData.size(), testData.size());
}

TEST(CallbackManager, ConcurrentCall)
{
    CallbackManager<> manager(0, true);
    size_t            count = 0;
    manager.AddCallback([&count]() { count++; });
    zeus::ThreadPool pool(6);
    const size_t     callbackCount = 1000000;
    zeus::Latch      callLatch(callbackCount);
    for (size_t i = 0; i < callbackCount; ++i)
    {
        pool.Commit(
            [&manager, &callLatch]()
            {
                manager.Call();
                callLatch.CountDown();
            }
        );
    }
    callLatch.Wait();
    EXPECT_LT(count, callbackCount);
}

TEST(NameCallbackManager, ConcurrentCall)
{
    NameCallbackManager<std::string> manager(0, true);
    size_t                           count = 0;
    manager.AddCallback("call", [&count]() { count++; });
    zeus::ThreadPool pool(6);
    const size_t     callbackCount = 1000000;
    zeus::Latch      callLatch(callbackCount);
    for (size_t i = 0; i < callbackCount; ++i)
    {
        pool.Commit(
            [&manager, &callLatch]()
            {
                manager.Call("call");
                callLatch.CountDown();
            }
        );
    }
    callLatch.Wait();
    EXPECT_LT(count, callbackCount);
}

TEST(CallbackManager, void)
{
    CallbackManager<> m(5);
    bool              flag = false;
    auto              id   = m.AddCallback([&flag]() { flag = true; });
    EXPECT_TRUE(id > 0);
    EXPECT_FALSE(flag);
    EXPECT_TRUE(m.Call());
    EXPECT_TRUE(flag);
    EXPECT_TRUE(m.RemoveCallback(id));
}

TEST(NameCallbackManager, void)
{
    NameCallbackManager<std::string> m(5);
    bool                             flag = false;
    auto                             id   = m.AddCallback("as", [&flag]() { flag = true; });
    EXPECT_TRUE(id > 0);
    EXPECT_FALSE(flag);
    EXPECT_TRUE(m.Call("as"));
    EXPECT_TRUE(flag);
    EXPECT_TRUE(m.RemoveCallback(id));
}

TEST(CallbackManager, rightValue)
{
    CallbackManager<MoveTest>     m(5);
    MoveTest                      flag;
    std::function<void(MoveTest)> callback = [&flag](MoveTest&& test)
    {
        flag = std::move(test);
    };
    auto id = m.AddCallback(std::move(callback));
    EXPECT_FALSE(callback);
    EXPECT_TRUE(id > 0);
    EXPECT_FALSE(flag.MoveConstruct());
    EXPECT_TRUE(m.Call(MoveTest()));
    EXPECT_TRUE(flag.MoveAssign());
    EXPECT_TRUE(m.RemoveCallback(id));
}

TEST(NameCallbackManager, rightValue)
{
    NameCallbackManager<std::string, MoveTest> m(5);
    MoveTest                                   flag;
    std::function<void(MoveTest)>              callback = [&flag](MoveTest&& test)
    {
        flag = std::move(test);
    };
    auto id = m.AddCallback("as", std::move(callback));
    EXPECT_FALSE(callback);
    EXPECT_TRUE(id > 0);
    EXPECT_FALSE(flag.MoveConstruct());
    EXPECT_TRUE(m.Call("as", MoveTest()));
    EXPECT_TRUE(flag.MoveAssign());
    EXPECT_TRUE(m.RemoveCallback(id));
}

TEST(CacheManager, base)
{
    const std::string kTestData1("TREDSSDSSTL:KKLJOIX");
    const std::string kTestData2("TREDSSDS5484459JOIX");
    const std::string kTestData3("TREDSmk892987IX");
    std::string       currentValue;
    size_t            createCount = 0;
    size_t            changeCount = 0;

    CacheManager<std::string> cache;
    cache.SetCreateCallback(
        [&createCount, kTestData1]()
        {
            ++createCount;
            return std::string(kTestData1);
        }
    );
    cache.SetChangeCallback(
        [&changeCount, &currentValue](const std::optional<const std::string>& value)
        {
            ++changeCount;
            if (value)
            {
                EXPECT_EQ(currentValue, *value);
            }
        }
    );

    currentValue = kTestData1;

    EXPECT_EQ(kTestData1, *cache.Get());
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(1, changeCount);
    EXPECT_FALSE(cache.Empty());

    currentValue = kTestData2;
    EXPECT_TRUE(cache.Set(kTestData2));
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(2, changeCount);
    EXPECT_FALSE(cache.Empty());
    EXPECT_EQ(kTestData2, *cache.Get());
    EXPECT_FALSE(cache.Set(kTestData2));
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(2, changeCount);
    EXPECT_FALSE(cache.Empty());

    EXPECT_TRUE(cache.Reset());
    EXPECT_EQ(3, changeCount);

    cache.Clear(true);
    EXPECT_TRUE(cache.Empty());
    EXPECT_EQ(3, changeCount);
}

TEST(MultiCacheManager, base)
{
    const std::string          kTestData1("TREDSSDSSTL:KKLJOIX");
    const std::string          kTestData2("TREDSSDS5484459JOIX");
    const std::string          kTestData3("TREDSmk892987IX");
    size_t                     createCount = 0;
    size_t                     changeCount = 0;
    std::optional<std::string> currentKey;
    std::optional<std::string> currentValue;
    bool                       changeCheck = true;

    MultiCacheManager<std::string, std::string> cache;
    cache.SetCreateCallback(
        [&createCount](const std::string& key)
        {
            ++createCount;
            return std::string(key + key);
        }
    );
    const auto changeCallback =
        [&currentKey, &changeCount, &currentValue, &changeCheck](const std::string& key, const std::optional<const std::string>& value)
    {
        ++changeCount;

        if (changeCheck)
        {
            EXPECT_EQ(currentKey, key);
            EXPECT_EQ(currentValue, value);
        }
    };
    cache.SetChangeCallback(changeCallback);
    currentKey   = kTestData1;
    currentValue = kTestData1 + kTestData1;
    EXPECT_EQ(kTestData1 + kTestData1, *cache.Get(kTestData1));
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(1, changeCount);
    EXPECT_FALSE(cache.Empty());
    EXPECT_EQ(1, cache.Size());

    currentKey   = kTestData2;
    currentValue = kTestData2 + kTestData2;
    cache.Set(kTestData2, std::string(kTestData2 + kTestData2));
    EXPECT_EQ(2, changeCount);
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(kTestData2 + kTestData2, *cache.Get(kTestData2));
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(2, changeCount);

    cache.Set(kTestData2, kTestData2 + kTestData2);
    EXPECT_EQ(2, changeCount);
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(kTestData2 + kTestData2, *cache.Get(kTestData2));
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(2, changeCount);

    cache.Set(kTestData2, kTestData2 + kTestData2, false);
    EXPECT_EQ(2, changeCount);
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(kTestData2 + kTestData2, *cache.Get(kTestData2));
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(2, changeCount);

    currentKey   = kTestData3;
    currentValue = kTestData3 + kTestData3;
    std::string temp(kTestData3 + kTestData3);
    cache.Set(kTestData3, temp);
    EXPECT_EQ(3, changeCount);
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(kTestData3 + kTestData3, *cache.Get(kTestData3));
    EXPECT_EQ(3, changeCount);
    EXPECT_EQ(1, createCount);

    currentKey   = kTestData3;
    currentValue = kTestData2 + kTestData2;
    cache.Set(kTestData3, kTestData2 + kTestData2);
    EXPECT_EQ(4, changeCount);
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(kTestData2 + kTestData2, *cache.Get(kTestData3));
    EXPECT_EQ(4, changeCount);
    EXPECT_EQ(1, createCount);
    EXPECT_EQ(3, cache.Size());

    changeCheck = false;
    cache.Notify(changeCallback);
    EXPECT_EQ(7, changeCount);

    changeCheck  = true;
    currentKey   = kTestData3;
    currentValue = std::nullopt;
    cache.Remove(kTestData3, true);

    EXPECT_EQ(8, changeCount);
    changeCheck = false;
    cache.Clear(true);
    EXPECT_TRUE(cache.Empty());
    EXPECT_EQ(10, changeCount);
    EXPECT_EQ(0, cache.Size());
}

TEST(CacheManager, neednotEqual)
{
    struct Test
    {
        std::string a;
        std::string b;
    };
    CacheManager<Test> cache;
    cache.SetCreateCallback([]() { return std::nullopt; });
    auto obj = cache.Get();
    EXPECT_FALSE(obj.has_value());
}

TEST(FixedBufferQueue, u8)
{
    static const size_t kTestCount = 25;
    using TestValue                = uint8_t;
    FixedBufferQueue<TestValue, std::mutex> buffer(kTestCount * 4);
    const size_t                            bytes = sizeof(decltype(buffer)::ValueType);
    EXPECT_TRUE(buffer.Empty());
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_EQ(buffer.BufferSize(), 0);
    EXPECT_EQ(buffer.Capacity(), kTestCount * 4);
    EXPECT_EQ(buffer.BufferCapacity(), kTestCount * 4 * bytes);
    std::array<TestValue, kTestCount * 8> data;
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = static_cast<TestValue>(i);
    }
    EXPECT_EQ(kTestCount, buffer.Push(data.data(), kTestCount));
    EXPECT_EQ(buffer.Size(), kTestCount);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount) *bytes);

    EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount, kTestCount));
    EXPECT_EQ(buffer.Size(), kTestCount * 2);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 2) * bytes);

    EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount * 2, kTestCount));
    EXPECT_EQ(buffer.Size(), kTestCount * 3);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 3) * bytes);

    EXPECT_EQ(0, buffer.Push(data.data() + kTestCount * 3, kTestCount + 1, false)); //这里故意越界，测试截断参数
    EXPECT_EQ(buffer.Size(), kTestCount * 3);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 3) * bytes);

    EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount * 3, kTestCount + 1, true)); //这里故意越界，测试截断参数
    EXPECT_EQ(buffer.Size(), kTestCount * 4);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 4) * bytes);

    EXPECT_EQ(buffer.Capacity(), kTestCount * 4);
    EXPECT_EQ(buffer.BufferCapacity(), kTestCount * 4 * bytes);

    EXPECT_FALSE(buffer.Empty());
    {
        size_t              start     = 0;
        static const size_t kPopCount = 10;
        for (size_t i = 0; i < buffer.Capacity() / kPopCount; i++)
        {
            auto temp = buffer.PopShared(kPopCount);
            EXPECT_EQ(buffer.Size(), buffer.Capacity() - (i + 1) * kPopCount);
            EXPECT_EQ(buffer.BufferSize(), buffer.BufferCapacity() - (i + 1) * kPopCount * bytes);
            for (size_t j = 0; j < kPopCount; ++j, ++start)
            {
                EXPECT_EQ(start, temp.get()[j]);
            }
        }
    }
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_EQ(buffer.BufferSize(), 0);
    {
        static const size_t              kPopCount = 30;
        std::array<TestValue, kPopCount> temp;
        EXPECT_EQ(kTestCount, buffer.Push(data.data(), kTestCount));
        EXPECT_EQ(buffer.Size(), kTestCount);
        EXPECT_EQ(buffer.BufferSize(), (kTestCount) *bytes);

        EXPECT_EQ(0, buffer.Pop(temp.data(), kPopCount, false));
        EXPECT_EQ(buffer.Size(), kTestCount);
        EXPECT_EQ(buffer.BufferSize(), (kTestCount) *bytes);

        EXPECT_EQ(kTestCount, buffer.Pop(temp.data(), kPopCount, true));
        EXPECT_EQ(buffer.Size(), 0);
        EXPECT_EQ(buffer.BufferSize(), 0);
    }
    {
        static const size_t              kPopCount  = kTestCount + 2;
        size_t                           expectSize = 0;
        size_t                           start      = 0;
        std::array<TestValue, kPopCount> temp;
        EXPECT_EQ(kTestCount, buffer.Push(data.data(), kTestCount));
        expectSize += kTestCount;
        EXPECT_EQ(buffer.Size(), expectSize);
        EXPECT_EQ(buffer.BufferSize(), (expectSize) *bytes);

        for (size_t c = 0; c < data.size() / kPopCount; ++c)
        {
            EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount * (c + 1), kTestCount));
            expectSize += kTestCount;
            EXPECT_EQ(buffer.Size(), expectSize);
            EXPECT_EQ(buffer.BufferSize(), (expectSize) *bytes);

            EXPECT_EQ(kPopCount, buffer.Pop(temp.data(), kPopCount, true));
            expectSize -= kPopCount;
            EXPECT_EQ(buffer.Size(), expectSize);
            EXPECT_EQ(buffer.BufferSize(), (expectSize) *bytes);
            for (size_t i = 0; i < kPopCount; ++i, ++start)
            {
                EXPECT_EQ(start, temp[i]);
            }
        }
    }
}

TEST(FixedBufferQueue, u32)
{
    static const size_t kTestCount = 25;
    using TestValue                = uint32_t;
    FixedBufferQueue<TestValue, std::mutex> buffer(kTestCount * 4);
    const size_t                            bytes = sizeof(decltype(buffer)::ValueType);
    EXPECT_TRUE(buffer.Empty());
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_EQ(buffer.BufferSize(), 0);
    EXPECT_EQ(buffer.Capacity(), kTestCount * 4);
    EXPECT_EQ(buffer.BufferCapacity(), kTestCount * 4 * bytes);

    std::array<TestValue, kTestCount * 8> data;
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = i;
    }
    EXPECT_EQ(kTestCount, buffer.Push(data.data(), kTestCount));
    EXPECT_EQ(buffer.Size(), kTestCount);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount) *bytes);

    EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount, kTestCount));
    EXPECT_EQ(buffer.Size(), kTestCount * 2);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 2) * bytes);

    EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount * 2, kTestCount));
    EXPECT_EQ(buffer.Size(), kTestCount * 3);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 3) * bytes);

    EXPECT_EQ(0, buffer.Push(data.data() + kTestCount * 3, kTestCount + 1, false)); //这里故意越界，测试截断参数
    EXPECT_EQ(buffer.Size(), kTestCount * 3);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 3) * bytes);

    EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount * 3, kTestCount + 1, true)); //这里故意越界，测试截断参数
    EXPECT_EQ(buffer.Size(), kTestCount * 4);
    EXPECT_EQ(buffer.BufferSize(), (kTestCount * 4) * bytes);

    EXPECT_EQ(buffer.Capacity(), kTestCount * 4);
    EXPECT_EQ(buffer.BufferCapacity(), kTestCount * 4 * bytes);
    EXPECT_FALSE(buffer.Empty());
    {
        size_t              start     = 0;
        static const size_t kPopCount = 10;
        for (size_t i = 0; i < buffer.Capacity() / kPopCount; i++)
        {
            auto temp = buffer.PopShared(kPopCount);
            EXPECT_EQ(buffer.Size(), buffer.Capacity() - (i + 1) * kPopCount);
            EXPECT_EQ(buffer.BufferSize(), buffer.BufferCapacity() - (i + 1) * kPopCount * bytes);
            for (size_t j = 0; j < kPopCount; ++j, ++start)
            {
                EXPECT_EQ(start, temp.get()[j]);
            }
        }
    }
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_EQ(buffer.BufferSize(), 0);
    {
        static const size_t              kPopCount = 30;
        std::array<TestValue, kPopCount> temp;
        EXPECT_EQ(kTestCount, buffer.Push(data.data(), kTestCount));
        EXPECT_EQ(buffer.Size(), kTestCount);
        EXPECT_EQ(buffer.BufferSize(), (kTestCount) *bytes);

        EXPECT_EQ(0, buffer.Pop(temp.data(), kPopCount, false));
        EXPECT_EQ(buffer.Size(), kTestCount);
        EXPECT_EQ(buffer.BufferSize(), (kTestCount) *bytes);

        EXPECT_EQ(kTestCount, buffer.Pop(temp.data(), kPopCount, true));
        EXPECT_EQ(buffer.Size(), 0);
        EXPECT_EQ(buffer.BufferSize(), 0);
    }
    {
        static const size_t              kPopCount  = kTestCount + 2;
        size_t                           expectSize = 0;
        size_t                           start      = 0;
        std::array<TestValue, kPopCount> temp;
        EXPECT_EQ(kTestCount, buffer.Push(data.data(), kTestCount));
        expectSize += kTestCount;
        EXPECT_EQ(buffer.Size(), expectSize);
        EXPECT_EQ(buffer.BufferSize(), (expectSize) *bytes);

        for (size_t c = 0; c < data.size() / kPopCount; ++c)
        {
            EXPECT_EQ(kTestCount, buffer.Push(data.data() + kTestCount * (c + 1), kTestCount));
            expectSize += kTestCount;
            EXPECT_EQ(buffer.Size(), expectSize);
            EXPECT_EQ(buffer.BufferSize(), (expectSize) *bytes);

            EXPECT_EQ(kPopCount, buffer.Pop(temp.data(), kPopCount, true));
            expectSize -= kPopCount;
            EXPECT_EQ(buffer.Size(), expectSize);
            EXPECT_EQ(buffer.BufferSize(), (expectSize) *bytes);
            for (size_t i = 0; i < kPopCount; ++i, ++start)
            {
                EXPECT_EQ(start, temp[i]);
            }
        }
    }
}

TEST(FilterManager, base)
{
    FilterManager<std::string> filter;
    auto                       id1 = filter.AddFilter([](const std::string& str) { return str.find("1") != std::string::npos; });
    auto                       id2 = filter.AddFilter([](const std::string& str) { return str.find("2") != std::string::npos; });
    auto                       id3 = filter.AddFilter([](const std::string& str) { return str.find("3") != std::string::npos; });
    EXPECT_TRUE(filter.CheckFilter("123"));
    EXPECT_TRUE(filter.CheckFilter("321123"));
    EXPECT_FALSE(filter.CheckFilter("4"));
    EXPECT_FALSE(filter.CheckFilter("5"));
    EXPECT_FALSE(filter.CheckFilter("6"));
    EXPECT_FALSE(filter.CheckFilter("12"));
    EXPECT_FALSE(filter.CheckFilter("23"));
    filter.RemoveFilter(id1);
    EXPECT_TRUE(filter.CheckFilter("23"));
    filter.Clear();
    EXPECT_TRUE(filter.CheckFilter("1"));
    EXPECT_TRUE(filter.CheckFilter("2"));
    EXPECT_TRUE(filter.CheckFilter("3"));
    EXPECT_TRUE(filter.CheckFilter("4"));
    EXPECT_TRUE(filter.CheckFilter("5"));
    EXPECT_TRUE(filter.CheckFilter("6"));
}