#pragma once
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace zeus
{

template<typename C>
std::list<C> VectorToList(const std::vector<C> &source)
{
    std::list<C> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item);
    }
    return dest;
}

template<typename C>
std::vector<C> ListToVector(const std::list<C> &source)
{
    std::vector<C> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item);
    }
    return dest;
}

template<typename C>
std::vector<C> SetToVector(const std::set<C> &source)
{
    std::vector<C> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item);
    }
    return dest;
}

template<typename C>
std::set<C> VectorToSet(const std::vector<C> &source)
{
    std::set<C> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename C>
std::list<C> SetToList(const std::set<C> &source)
{
    std::list<C> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item);
    }
    return dest;
}

template<typename C>
std::set<C> ListToSet(const std::list<C> &source)
{
    std::set<C> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename C>
std::vector<C> SetToVector(const std::unordered_set<C> &source)
{
    std::vector<C> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item);
    }
    return dest;
}

template<typename C>
std::unordered_set<C> VectorToUnorderedSet(const std::vector<C> &source)
{
    std::unordered_set<C> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename C>
std::list<C> SetToList(const std::unordered_set<C> &source)
{
    std::list<C> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item);
    }
    return dest;
}

template<typename C>
std::unordered_set<C> ListToUnorderedSet(const std::list<C> &source)
{
    std::unordered_set<C> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename C>
std::set<C> SetToSet(const std::unordered_set<C> &source)
{
    std::set<C> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename C>
std::unordered_set<C> SetToSet(const std::set<C> &source)
{
    std::unordered_set<C> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename K, typename V>
std::unordered_map<K, V> MapToMap(const std::map<K, V> &source)
{
    std::unordered_map<K, V> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename K, typename V>
std::map<K, V> MapToMap(const std::unordered_map<K, V> &source)
{
    std::map<K, V> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename K, typename V>
std::unordered_multimap<K, V> MapToMap(const std::multimap<K, V> &source)
{
    std::unordered_multimap<K, V> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename K, typename V>
std::multimap<K, V> MapToMap(const std::unordered_multimap<K, V> &source)
{
    std::multimap<K, V> dest;
    for (auto &item : source)
    {
        dest.emplace(item);
    }
    return dest;
}

template<typename K, typename V>
std::vector<K> MapKeysToVector(const std::map<K, V> &source)
{
    std::vector<K> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::list<K> MapKeysToList(const std::map<K, V> &source)
{
    std::list<K> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::set<K> MapKeysToSet(const std::map<K, V> &source)
{
    std::set<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::unordered_set<K> MapKeysToUnorderedSet(const std::map<K, V> &source)
{
    std::unordered_set<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::vector<K> MapKeysToVector(const std::multimap<K, V> &source)
{
    std::vector<K> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::list<K> MapKeysToList(const std::multimap<K, V> &source)
{
    std::list<K> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::multiset<K> MapKeysToSet(const std::multimap<K, V> &source)
{
    std::multiset<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::unordered_multiset<K> MapKeysToUnorderedSet(const std::multimap<K, V> &source)
{
    std::unordered_multiset<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::vector<K> MapKeysToVector(const std::unordered_map<K, V> &source)
{
    std::vector<K> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::list<K> MapKeysToList(const std::unordered_map<K, V> &source)
{
    std::list<K> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::set<K> MapKeysToSet(const std::unordered_map<K, V> &source)
{
    std::set<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::unordered_set<K> MapKeysToUnorderedSet(const std::unordered_map<K, V> &source)
{
    std::unordered_set<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::vector<K> MapKeysToVector(const std::unordered_multimap<K, V> &source)
{
    std::vector<K> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::list<V> MapKeysToList(const std::unordered_multimap<K, V> &source)
{
    std::list<V> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::multiset<K> MapKeysToSet(const std::unordered_multimap<K, V> &source)
{
    std::multiset<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::unordered_multiset<K> MapKeysToUnorderedSet(const std::unordered_multimap<K, V> &source)
{
    std::unordered_multiset<K> dest;
    for (auto &item : source)
    {
        dest.emplace(item.first);
    }
    return dest;
}

template<typename K, typename V>
std::vector<V> MapValuesToVector(const std::map<K, V> &source)
{
    std::vector<V> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::list<V> MapValuesToList(const std::map<K, V> &source)
{
    std::list<V> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::vector<V> MapValuesToVector(const std::multimap<K, V> &source)
{
    std::vector<V> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::list<V> MapValuesToList(const std::multimap<K, V> &source)
{
    std::list<V> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::vector<V> MapValuesToVector(const std::unordered_map<K, V> &source)
{
    std::vector<V> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::list<V> MapValuesToList(const std::unordered_map<K, V> &source)
{
    std::list<V> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::vector<V> MapValuesToVector(const std::unordered_multimap<K, V> &source)
{
    std::vector<V> dest;
    dest.reserve(source.size());
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename K, typename V>
std::list<V> MapValuesToList(const std::unordered_multimap<K, V> &source)
{
    std::list<V> dest;
    for (auto &item : source)
    {
        dest.emplace_back(item.second);
    }
    return dest;
}

template<typename OldK, typename OldV, typename K, typename V>
std::map<K, V> MapToMap(
    const std::map<OldK, OldV> &source, const std::function<K(const OldK &)> &Khandler, const std::function<V(const OldV &)> &Vhandler
)
{
    std::map<K, V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Khandler(item.first), Vhandler(item.second));
    }
    return dest;
}

template<typename OldK, typename OldV, typename K, typename V>
std::multimap<K, V> MapToMap(
    const std::multimap<OldK, OldV> &source, const std::function<K(const OldK &)> &Khandler, const std::function<V(const OldV &)> &Vhandler
)
{
    std::multimap<K, V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Khandler(item.first), Vhandler(item.second));
    }
    return dest;
}

template<typename OldK, typename OldV, typename K, typename V>
std::unordered_map<K, V> MapToMap(
    const std::unordered_map<OldK, OldV> &source, const std::function<K(const OldK &)> &Khandler, const std::function<V(const OldV &)> &Vhandler
)
{
    std::unordered_map<K, V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Khandler(item.first), Vhandler(item.second));
    }
    return dest;
}

template<typename OldK, typename OldV, typename K, typename V>
std::unordered_multimap<K, V> MapToMap(
    const std::unordered_multimap<OldK, OldV> &source, const std::function<K(const OldK &)> &Khandler, const std::function<V(const OldV &)> &Vhandler
)
{
    std::unordered_multimap<K, V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Khandler(item.first), Vhandler(item.second));
    }
    return dest;
}

template<typename OldV, typename V>
std::set<V> SetToSet(const std::set<OldV> &source, const std::function<V(const OldV &)> &Vhandler)
{
    std::set<V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Vhandler(item));
    }
    return dest;
}

template<typename OldV, typename V>
std::multiset<V> SetToSet(const std::multiset<OldV> &source, const std::function<V(const OldV &)> &Vhandler)
{
    std::multiset<V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Vhandler(item));
    }
    return dest;
}

template<typename OldV, typename V>
std::unordered_set<V> SetToSet(const std::unordered_set<OldV> &source, const std::function<V(const OldV &)> &Vhandler)
{
    std::unordered_set<V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Vhandler(item));
    }
    return dest;
}

template<typename OldV, typename V>
std::unordered_multiset<V> SetToSet(const std::unordered_multiset<OldV> &source, const std::function<V(const OldV &)> &Vhandler)
{
    std::unordered_multiset<V> dest;
    for (const auto &item : source)
    {
        dest.emplace(Vhandler(item));
    }
    return dest;
}

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
