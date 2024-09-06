#include "zeus/foundation/serialization/file_serializer.h"
#include <fstream>
#include <list>
#include <cstring>
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/file/file_utils.h"

namespace zeus
{
struct FileSerializerImpl
{
    std::filesystem::path        path;
    std::shared_ptr<BaseDigest>  digest;
    std::shared_ptr<BaseEncrypt> encrypt;
    std::shared_ptr<BaseDecrypt> decrypt;
};
FileSerializer::FileSerializer(const std::filesystem::path& path) : _impl(std::make_unique<FileSerializerImpl>())
{
    _impl->path = path;
}

FileSerializer::~FileSerializer()
{
}
void FileSerializer::SetCrypt(const std::shared_ptr<BaseEncrypt>& encrypt, const std::shared_ptr<BaseDecrypt>& decrypt)
{
    _impl->encrypt = encrypt;
    _impl->decrypt = decrypt;
}
void FileSerializer::SetDigest(const std::shared_ptr<BaseDigest>& digest)
{
    _impl->digest = digest;
}
zeus::expected<std::vector<uint8_t>, SerializerError> FileSerializer::Load()
{
    std::ifstream file(_impl->path, std::ios::binary);
    if (!file)
    {
        return zeus::unexpected(SerializerError::kFileIoError);
    }
    std::error_code ec;
    const auto      fileSize = std::filesystem::file_size(_impl->path, ec);
    if (ec)
    {
        return zeus::unexpected(SerializerError::kFileIoError);
    }
    auto fileBuffer = std::make_unique<char[]>(fileSize);
    file.read(fileBuffer.get(), fileSize);
    if (fileSize != file.gcount())
    {
        return zeus::unexpected(SerializerError::kFileIoError);
    }
    zeus::AutoRelease reset(
        [this]()
        {
            if (_impl->digest)
            {
                _impl->digest->Reset();
            }
            if (_impl->decrypt)
            {
                _impl->decrypt->Reset();
            }
        }
    );
    const std::byte* contentData = reinterpret_cast<std::byte*>(fileBuffer.get());
    size_t           contentSize = fileSize;
    if (_impl->digest)
    {
        const auto digestName = _impl->digest->Name();
        if (contentSize <= digestName.size() + _impl->digest->GetSize())
        {
            return zeus::unexpected(SerializerError::kDigestFileSizeUnsatisfied);
        }
        if (0 != std::memcmp(digestName.data(), contentData, digestName.size()))
        {
            return zeus::unexpected(SerializerError::kDigestNameUnmatched);
        }
        _impl->digest->Update(contentData + digestName.size() + _impl->digest->GetSize(), contentSize - digestName.size() - _impl->digest->GetSize());
        if (0 != std::memcmp(_impl->digest->Digest(), contentData + digestName.size(), _impl->digest->GetSize()))
        {
            return zeus::unexpected(SerializerError::kDigestValidationFailed);
        }
        contentData += digestName.size() + _impl->digest->GetSize();
        contentSize -= digestName.size() + _impl->digest->GetSize();
    }
    if (_impl->decrypt)
    {
        const auto decryptName = _impl->decrypt->Name();
        if (contentSize <= decryptName.size())
        {
            return zeus::unexpected(SerializerError::kCryptFileSizeUnsatisfied);
        }
        if (0 != std::memcmp(decryptName.data(), contentData, decryptName.size()))
        {
            return zeus::unexpected(SerializerError::kCryptNameUnmatched);
        }
        _impl->decrypt->Update(contentData + decryptName.size(), contentSize - decryptName.size());
        _impl->decrypt->End();
        contentData = _impl->decrypt->PlainText();
        contentSize = _impl->decrypt->GetSize();
    }
    std::vector<uint8_t> result;
    if (contentSize)
    {
        result.resize(contentSize);
        std::memcpy(result.data(), contentData, contentSize);
    }
    return std::move(result);
}
zeus::expected<void, SerializerError> FileSerializer::Save(const void* buffer, size_t bufferSize)
{
    std::list<std::pair<const void*, size_t>> contentList;
    std::string                               digestName;
    std::string                               encryptName;
    contentList.emplace_front(buffer, bufferSize);
    zeus::AutoRelease reset(
        [this]()
        {
            if (_impl->digest)
            {
                _impl->digest->Reset();
            }
            if (_impl->encrypt)
            {
                _impl->encrypt->Reset();
            }
        }
    );
    if (_impl->encrypt)
    {
        encryptName = _impl->encrypt->Name();
        for (auto& content : contentList)
        {
            _impl->encrypt->Update(content.first, content.second);
        }
        _impl->encrypt->End();
        contentList.clear();
        contentList.emplace_front(_impl->encrypt->CipherText(), _impl->encrypt->GetSize());
        contentList.emplace_front(encryptName.data(), encryptName.size());
    }
    if (_impl->digest)
    {
        digestName = _impl->digest->Name();
        for (auto& content : contentList)
        {
            _impl->digest->Update(content.first, content.second);
        }
        contentList.emplace_front(_impl->digest->Digest(), _impl->digest->GetSize());
        contentList.emplace_front(digestName.data(), digestName.size());
    }
    std::ofstream file(_impl->path, std::ios::binary);
    if (!file)
    {
        CreateWriteableDirectory(_impl->path.parent_path());
        file.open(_impl->path, std::ios::binary);
    }
    if (!file)
    {
        return zeus::unexpected(SerializerError::kFileIoError);
    }
    for (auto& content : contentList)
    {
        file.write(static_cast<const char*>(content.first), content.second);
        if (file.bad())
        {
            return zeus::unexpected(SerializerError::kFileIoError);
        }
    }
    file.close();
    if (file.bad())
    {
        return zeus::unexpected(SerializerError::kFileIoError);
    }
    return {};
}
} // namespace zeus
