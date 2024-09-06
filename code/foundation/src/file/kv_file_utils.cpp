#include "zeus/foundation/file/kv_file_utils.h"

#include <fstream>
#include <string_view>
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{

zeus::expected<std::string, std::error_code> GetKVFileValue(const std::filesystem::path& path, std::string_view key, std::string_view delim)
{
    std::string value;
    auto        result = FileEachLineKVData(
        path, delim,
        [&key, &value](std::string_view k, std::string_view v)
        {
            if (key == k)
            {
                value = v;
                return false;
            }

            return true;
        }
    );
    if (result)
    {
        return value;
    }
    return zeus::unexpected(result.error());
}

zeus::expected<std::vector<std::string>, std::error_code> GetKVFileValues(
    const std::filesystem::path& path, std::string_view key, std::string_view delim
)
{
    std::vector<std::string> values;
    auto                     result = FileEachLineKVData(
        path, delim,
        [&key, &values](std::string_view k, std::string_view v)
        {
            if (key == k)
            {
                values.emplace_back(v);
            }
            return true;
        }
    );
    if (result)
    {
        return values;
    }
    return zeus::unexpected(result.error());
}

zeus::expected<std::string, std::error_code> GetKVFileValueUnquoted(const std::filesystem::path& path, std::string_view key, std::string_view delim)
{
    const auto quotedValue = GetKVFileValue(path, key, delim);
    if (quotedValue)
    {
        return Unquote(quotedValue.value());
    }
    return zeus::unexpected(quotedValue.error());
}

zeus::expected<std::multimap<std::string, std::string>, std::error_code> GetKVFileData(const std::filesystem::path& path, std::string_view delim)
{
    std::multimap<std::string, std::string> values;
    auto                                    result = FileEachLineKVData(
        path, delim,
        [&values](std::string_view k, std::string_view v)
        {
            values.emplace(k, v);
            return true;
        }
    );
    if (result)
    {
        return values;
    }
    return zeus::unexpected(result.error());
}

zeus::expected<std::map<std::string, std::string>, std::error_code> GetKVFileUniqueData(const std::filesystem::path& path, std::string_view delim)
{
    std::map<std::string, std::string> values;
    auto                               result = FileEachLineKVData(
        path, delim,
        [&values](std::string_view k, std::string_view v)
        {
            if (values.find(std::string(k)) == values.end())
            {
                values.emplace(k, v);
            }
            return true;
        }
    );
    if (result)
    {
        return values;
    }
    return zeus::unexpected(result.error());
}

zeus::expected<std::multimap<std::string, std::string>, std::error_code> PickKVFileData(
    const std::filesystem::path& path, const std::set<std::string>& keys, std::string_view delim
)
{
    std::multimap<std::string, std::string> values;
    auto                                    result = FileEachLineKVData(
        path, delim,
        [&keys, &values](std::string_view k, std::string_view v)
        {
            if (keys.find(std::string(k)) != keys.end())
            {
                values.emplace(k, v);
            }
            return true;
        }
    );
    if (result)
    {
        return values;
    }
    return zeus::unexpected(result.error());
}
zeus::expected<std::map<std::string, std::string>, std::error_code> PickKVFileUniqueData(
    const std::filesystem::path& path, const std::set<std::string>& keys, std::string_view delim
)
{
    std::map<std::string, std::string> values;
    auto                               result = FileEachLineKVData(
        path, delim,
        [&keys, &values](std::string_view k, std::string_view v)
        {
            if (keys.find(std::string(k)) != keys.end())
            {
                values.emplace(k, v);
                if (values.size() == keys.size())
                {
                    return false;
                }
            }
            return true;
        }
    );
    if (result)
    {
        return values;
    }
    return zeus::unexpected(result.error());
}

} // namespace zeus
