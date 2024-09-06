#ifdef _WIN32
#include "zeus/foundation/system/win/file_icon.h"
#include <array>
#include <fstream>
#include <Windows.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/system/shared_library.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/byte/byte_order.h"

namespace zeus
{
namespace
{
#pragma pack(push)
#pragma pack(2)
struct MemoryIconDirEntry
{
    uint8_t  width;      // Width of the image
    uint8_t  height;     // Height of the image (times 2)
    uint8_t  colorCount; // Number of colors in image (0 if >=8bpp)
    uint8_t  reserved;   // Reserved
    uint16_t planes;     // Color Planes
    uint16_t bitCount;   // Bits per pixel
    uint32_t bytesInRes; // how many bytes in this resource?
    uint16_t nID;        // the ID
};
struct IconDir
{
    uint16_t idReserved; // Reserved
    uint16_t idType;     // resource type (1 for icons)
    uint16_t idCount;    // how many images?
};

struct IconDirEntry
{
    uint8_t  width;      // Width of the image
    uint8_t  height;     // Height of the image (times 2)
    uint8_t  colorCount; // Number of colors in image (0 if >=8bpp)
    uint8_t  reserved;   // Reserved
    uint16_t planes;     // Color Planes
    uint16_t bitCount;   // Bits per pixel
    uint32_t bytesInRes; // how many bytes in this resource?
    uint32_t offset;     // the offset
};

struct PNG_IHDR
{
    uint32_t length;
    uint8_t  type[4];
    uint32_t width;
    uint32_t height;
};

constexpr size_t kMaxIconSize = 256;

constexpr std::array<uint8_t, 8> kPNGCheck = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

constexpr std::array<uint8_t, 4> kIHDRCheck = {'I', 'H', 'D', 'R'};

#pragma pack(pop)

bool IsPng(const void* data, size_t length)
{
    if (length <= kPNGCheck.size() + sizeof(PNG_IHDR))
    {
        return false;
    }
    if (0 != std::memcmp(kPNGCheck.data(), data, kPNGCheck.size()))
    {
        return false;
    }
    const auto* ihdr = reinterpret_cast<const PNG_IHDR*>(static_cast<const uint8_t*>(data) + kPNGCheck.size());
    if (length <= kPNGCheck.size() + sizeof(PNG_IHDR::length) + sizeof(PNG_IHDR::type) + FlipBytes(ihdr->length))
    {
        return false;
    }
    return 0 == std::memcmp(ihdr->type, kIHDRCheck.data(), kIHDRCheck.size());
}

std::pair<uint32_t, uint32_t> GetPNGSize(const void* data)
{
    const auto* ihdr = reinterpret_cast<const PNG_IHDR*>(static_cast<const uint8_t*>(data) + kPNGCheck.size());
    return {FlipBytes(ihdr->width), FlipBytes(ihdr->height)};
}

void WriteIcon(std::ofstream& file, const std::vector<const WinFileIconImage*>& items)
{
    IconDir iconDir = {0, 1, items.size()};
    file.write(reinterpret_cast<char*>(&iconDir), sizeof(IconDir));
    auto dataOffset = sizeof(IconDir) + sizeof(IconDirEntry) * items.size();
    for (const auto* item : items)
    {
        IconDirEntry entry = {
            static_cast<uint8_t>(item->EntryWidth()),
            static_cast<uint8_t>(item->EntryHeight()),
            item->ColorCount(),
            0,
            item->Planes(),
            item->BitCount(),
            static_cast<uint32_t>(item->Size()),
            dataOffset};
        file.write(reinterpret_cast<char*>(&entry), sizeof(entry));
        dataOffset += item->Size();
    }
    for (const auto* item : items)
    {
        file.write(static_cast<const char*>(item->Data()), item->Size());
    }
}

} // namespace

struct WinFileIconImageImpl
{
    size_t                         width       = 0;
    size_t                         height      = 0;
    size_t                         entryWidth  = 0;
    size_t                         entryHeight = 0;
    bool                           standard    = true;
    uint16_t                       planes      = 0;
    uint16_t                       bitCount    = 0;
    uint8_t                        colorCount  = 0;
    uint16_t                       id          = 0;
    void*                          data        = nullptr;
    size_t                         size        = 0;
    WinFileIconImage::IconItemType type        = WinFileIconImage::IconItemType::kBmp;
};

WinFileIconImage::WinFileIconImage() : _impl(std::make_unique<WinFileIconImageImpl>())
{
}

WinFileIconImage::~WinFileIconImage()
{
}

WinFileIconImage::WinFileIconImage(WinFileIconImage&& other) noexcept : _impl(std::move(other._impl))
{
}

WinFileIconImage& WinFileIconImage::operator=(WinFileIconImage&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

const void* WinFileIconImage::Data() const noexcept
{
    return _impl->data;
}

size_t WinFileIconImage::Size() const noexcept
{
    return _impl->size;
}

size_t WinFileIconImage::Width() const noexcept
{
    return _impl->width;
}

size_t WinFileIconImage::Height() const noexcept
{
    return _impl->height;
}

size_t WinFileIconImage::EntryWidth() const noexcept
{
    return _impl->entryWidth;
}

size_t WinFileIconImage::EntryHeight() const noexcept
{
    return _impl->entryHeight;
}

uint8_t zeus::WinFileIconImage::ColorCount() const noexcept
{
    return _impl->colorCount;
}

uint16_t zeus::WinFileIconImage::Planes() const noexcept
{
    return _impl->planes;
}

uint16_t zeus::WinFileIconImage::BitCount() const noexcept
{
    return _impl->bitCount;
}

uint16_t zeus::WinFileIconImage::Id() const noexcept
{
    return _impl->id;
}

WinFileIconImage::IconItemType WinFileIconImage::Type() const noexcept
{
    return _impl->type;
}

bool WinFileIconImage::IsStandard() const noexcept
{
    return _impl->standard;
}

zeus::expected<void, std::error_code> WinFileIconImage::ExtractImage(const std::filesystem::path& path) const
{
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    file.write(static_cast<const char*>(_impl->data), _impl->size);
    return {};
}
zeus::expected<void, std::error_code> WinFileIconImage::ExtractAsIcon(const std::filesystem::path& path) const
{
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WriteIcon(file, {this});
    return {};
}

struct WinFileIconImpl
{
    std::vector<WinFileIconImage> items;
    std::string                   resourceId;
};

WinFileIcon::WinFileIcon() : _impl(std::make_unique<WinFileIconImpl>())
{
}
WinFileIcon::~WinFileIcon()
{
}
WinFileIcon::WinFileIcon(WinFileIcon&& other) noexcept : _impl(std::move(other._impl))
{
}

WinFileIcon& WinFileIcon::operator=(WinFileIcon&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

const std::vector<WinFileIconImage>& WinFileIcon::Images() const noexcept
{
    return _impl->items;
}

std::string WinFileIcon::ResourceId() const
{
    return _impl->resourceId;
}

zeus::expected<void, std::error_code> WinFileIcon::ExtractIconToFile(const std::filesystem::path& path, bool excludeUnStandard) const
{
    std::vector<const WinFileIconImage*> items;
    items.reserve(_impl->items.size());
    for (const auto& item : _impl->items)
    {
        if (excludeUnStandard && !item.IsStandard())
        {
            continue;
        }
        items.emplace_back(&item);
    }
    if (items.empty())
    {
        return zeus::unexpected(SystemError {ERROR_NO_DATA});
    }
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WriteIcon(file, items);
    return {};
}

struct WinFileIconGroupImpl
{
    zeus::SharedLibrary      module {nullptr};
    std::vector<WinFileIcon> items;
};

WinFileIconGroup::WinFileIconGroup() : _impl(std::make_unique<WinFileIconGroupImpl>())
{
}

WinFileIconGroup::~WinFileIconGroup()
{
}

WinFileIconGroup::WinFileIconGroup(WinFileIconGroup&& other) noexcept : _impl(std::move(other._impl))
{
}

WinFileIconGroup& WinFileIconGroup::operator=(WinFileIconGroup&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

const std::vector<WinFileIcon>& WinFileIconGroup::Icons() const noexcept
{
    return _impl->items;
}

zeus::expected<WinFileIconGroup, std::error_code> zeus::WinFileIconGroup::Open(const std::filesystem::path& path)
{
    const auto EnumIconProc = [](HMODULE hModule, LPCWSTR /*lpszType*/, LPWSTR lpszName, LONG_PTR lParam) -> BOOL
    {
        std::vector<WinFileIcon>* items = reinterpret_cast<std::vector<WinFileIcon>*>(lParam);
        std::string               reourceId;
        if (IS_INTRESOURCE(lpszName))
        {
            reourceId = std::to_string(reinterpret_cast<ULONG_PTR>(lpszName));
        }
        else
        {
            reourceId = CharsetUtils::UnicodeToUTF8(lpszName);
        }

        HRSRC iconSrc = ::FindResourceW(hModule, lpszName, RT_GROUP_ICON);
        if (!iconSrc)
        {
            return TRUE;
        }
        HGLOBAL iconGlobal = ::LoadResource(hModule, iconSrc);
        if (!iconGlobal)
        {
            return TRUE;
        }
        const auto* iconDir = static_cast<IconDir*>(::LockResource(iconGlobal));
        if (!iconDir)
        {
            return TRUE;
        }
        if (0 != iconDir->idReserved)
        {
            return TRUE;
        }
        if (1 != iconDir->idType)
        {
            return TRUE;
        }
        if (!iconDir->idCount)
        {
            return TRUE;
        }
        WinFileIcon icon;
        icon._impl->resourceId = std::move(reourceId);
        icon._impl->items.reserve(iconDir->idCount);
        const auto* idEntries = reinterpret_cast<const MemoryIconDirEntry*>(reinterpret_cast<const uint8_t*>(iconDir) + sizeof(IconDir));
        for (auto index = 0; index < iconDir->idCount; ++index)
        {
            const MemoryIconDirEntry& entry    = idEntries[index];
            HRSRC                     imageSrc = ::FindResourceW(hModule, MAKEINTRESOURCEW(entry.nID), RT_ICON);
            if (!imageSrc)
            {
                continue;
            }
            HGLOBAL imageGlobal = ::LoadResource(hModule, imageSrc);
            auto*   imageData   = static_cast<uint8_t*>(::LockResource(imageGlobal));
            if (!imageData)
            {
                continue;
            }
            if (!entry.bytesInRes)
            {
                continue;
            }
            WinFileIconImage image;
            image._impl->width       = entry.width;
            image._impl->height      = entry.height;
            image._impl->entryWidth  = entry.width;
            image._impl->entryHeight = entry.height;
            image._impl->planes      = entry.planes;
            image._impl->bitCount    = entry.bitCount;
            image._impl->colorCount  = entry.colorCount;
            image._impl->id          = entry.nID;
            image._impl->size        = SizeofResource(hModule, imageSrc);
            image._impl->data        = imageData;
            if (IsPng(imageData, image._impl->size))
            {
                image._impl->type    = WinFileIconImage::IconItemType::kPng;
                auto [width, height] = GetPNGSize(imageData);

                if (width != height || width > kMaxIconSize || height > kMaxIconSize)
                {
                    image._impl->standard = false;
                }
                image._impl->width  = width;
                image._impl->height = height;
            }
            else
            {
                image._impl->type = WinFileIconImage::IconItemType::kBmp;
            }
            icon._impl->items.emplace_back(std::move(image));
        }
        items->emplace_back(std::move(icon));
        return TRUE;
    };
    auto module = zeus::SharedLibrary::Load(path, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (!module.has_value())
    {
        return zeus::unexpected(module.error());
    }
    std::vector<WinFileIcon> items;
    if (!::EnumResourceNamesW(
            module.value(), RT_GROUP_ICON, static_cast<BOOL(__stdcall*)(HMODULE, LPCWSTR, LPWSTR, LONG_PTR)>(EnumIconProc),
            reinterpret_cast<LONG_PTR>(&items)
        ))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    if (items.empty())
    {
        return zeus::unexpected(SystemError {ERROR_NO_DATA});
    }
    WinFileIconGroup group;
    group._impl->module = std::move(module.value());
    group._impl->items  = std::move(items);
    return group;
}

} // namespace zeus

#endif //_WIN32
