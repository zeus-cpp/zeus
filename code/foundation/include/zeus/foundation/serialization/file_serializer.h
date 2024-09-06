#pragma once
#include <memory>
#include <filesystem>
#include "zeus/foundation/serialization/serializer.h"
#include "zeus/foundation/crypt/base_digest.h"
#include "zeus/foundation/crypt/base_encrypt.h"
#include "zeus/foundation/crypt/base_decrypt.h"

namespace zeus
{

struct FileSerializerImpl;
class FileSerializer : public Serializer
{
public:
    FileSerializer(const std::filesystem::path& path);
    ~FileSerializer() override;
    FileSerializer(const FileSerializer&)            = delete;
    FileSerializer& operator=(const FileSerializer&) = delete;
    FileSerializer(FileSerializer&&)                 = delete;
    FileSerializer& operator=(FileSerializer&&)      = delete;

    //如果都设置先加解密，再散列校验
    void SetCrypt(const std::shared_ptr<BaseEncrypt>& encrypt, const std::shared_ptr<BaseDecrypt>& decrypt);
    void SetDigest(const std::shared_ptr<BaseDigest>& digest);

    zeus::expected<std::vector<uint8_t>, SerializerError> Load() override;
    zeus::expected<void, SerializerError>                 Save(const void* buffer, size_t bufferSize) override;

private:
    std::unique_ptr<FileSerializerImpl> _impl;
};

}

#include "zeus/foundation/core/zeus_compatible.h"
