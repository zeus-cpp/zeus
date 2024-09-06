#include "zeus/foundation/file/ini_file_utils.h"
#include <functional>
#include <fstream>
#include <string_view>
#include "zeus/foundation/file/file_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{

namespace
{

zeus::expected<void, std::error_code> FileEachSectionData(
    const std::filesystem::path& path, std::string_view section, const std::function<bool(std::string_view key, std::string_view value)>& callback
)
{
    bool sectionHandleing = false;
    return FileEachLineKVData(
        path, "=",
        [&callback, &sectionHandleing](std::string_view key, std::string_view value)
        {
            if (!sectionHandleing)
            {
                return true;
            }
            return callback(Trim(key), value);
        },
        [&section, &sectionHandleing](std::string_view line)
        {
            if (line.size() >= 2 && '[' == line.front() && ']' == line.back())
            {
                line.remove_prefix(1);
                line.remove_suffix(1);
                if (section == line)
                {
                    sectionHandleing = true;
                    return true;
                }
                else
                {
                    sectionHandleing = false;
                    return false;
                }
            }
            return true;
        },
        {";", "#"}
    );
}
} // namespace

zeus::expected<std::string, std::error_code> GetIniFileValue(
    const std::filesystem::path& path, const std::string_view& section, const std::string_view& key
)
{
    std::string value;
    auto        result = FileEachSectionData(
        path, section,
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

zeus::expected<std::vector<std::string>, std::error_code> GetIniFileValues(
    const std::filesystem::path& path, const std::string_view& section, const std::string_view& key
)
{
    std::vector<std::string> values;
    auto                     result = FileEachSectionData(
        path, section,
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

zeus::expected<std::set<std::string>, std::error_code> GetIniFileSections(const std::filesystem::path& path)
{
    std::set<std::string> sections;
    auto                  result = FileEachLine(
        path,
        [&sections](std::string_view line)
        {
            if (line.size() >= 2 && '[' == line.front() && ']' == line.back())
            {
                line.remove_prefix(1);
                line.remove_suffix(1);
                sections.emplace(line);
            }
            return true;
        }
    );
    if (result)
    {
        return sections;
    }
    return zeus::unexpected(result.error());
}

zeus::expected<std::multimap<std::string, std::string>, std::error_code> GetIniFileData(
    const std::filesystem::path& path, const std::string_view& section
)
{
    std::multimap<std::string, std::string> values;
    auto                                    result = FileEachSectionData(
        path, section,
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

zeus::expected<std::map<std::string, std::string>, std::error_code> GetIniFileUniqueData(
    const std::filesystem::path& path, const std::string_view& section
)
{
    std::map<std::string, std::string> values;
    auto                               result = FileEachSectionData(
        path, section,
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

} // namespace zeus
