#include "zeus/foundation/file/file_wrapper.h"

namespace zeus
{

zeus::expected<size_t, std::error_code> FileWrapper::Write(const void* data, size_t size, int64_t offset, OffsetType type)
{
    if (auto ret = Seek(offset, type); !ret.has_value())
    {
        return zeus::unexpected(ret.error());
    }
    return Write(data, size);
}
zeus::expected<size_t, std::error_code> FileWrapper::Write(const std::string& data)
{
    return Write(data.data(), data.size());
}
zeus::expected<size_t, std::error_code> FileWrapper::Write(const std::string& data, int64_t offset, OffsetType type)
{
    return Write(data.data(), data.size(), offset, type);
}
zeus::expected<size_t, std::error_code> FileWrapper::Write(std::string_view data)
{
    return Write(data.data(), data.size());
}
zeus::expected<size_t, std::error_code> FileWrapper::Write(std::string_view data, int64_t offset, OffsetType type)
{
    return Write(data.data(), data.size(), offset, type);
}
zeus::expected<size_t, std::error_code> FileWrapper::Read(void* buffer, size_t size, int64_t offset, OffsetType type)
{
    if (auto ret = Seek(offset, type); !ret.has_value())
    {
        return zeus::unexpected(ret.error());
    }
    return Read(buffer, size);
}

zeus::expected<std::vector<uint8_t>, std::error_code> FileWrapper::Read(size_t size)
{
    std::vector<uint8_t> buffer(size);
    if (auto len = Read(buffer.data(), size); len.has_value())
    {
        buffer.resize(len.value());
        return buffer;
    }
    else
    {
        return zeus::unexpected(len.error());
    }
}

zeus::expected<std::vector<uint8_t>, std::error_code> FileWrapper::Read(size_t size, int64_t offset, OffsetType type)
{
    if (auto ret = Seek(offset, type); !ret.has_value())
    {
        return zeus::unexpected(ret.error());
    }
    std::vector<uint8_t> buffer(size);
    if (auto len = Read(buffer.data(), size); len.has_value())
    {
        buffer.resize(len.value());
        return buffer;
    }
    else
    {
        return zeus::unexpected(len.error());
    }
}

zeus::expected<std::string, std::error_code> FileWrapper::ReadString(size_t size)
{
    std::string buffer(size, '\0');
    if (auto len = Read(buffer.data(), size); len.has_value())
    {
        buffer.resize(len.value());
        return buffer;
    }
    else
    {
        return zeus::unexpected(len.error());
    }
}
zeus::expected<std::string, std::error_code> FileWrapper::ReadString(size_t size, int64_t offset, OffsetType type)
{
    if (auto ret = Seek(offset, type); !ret.has_value())
    {
        return zeus::unexpected(ret.error());
    }
    return ReadString(size);
}

} // namespace zeus
