#include "zeus/foundation/ipc/shared_memory.h"
#ifdef _WIN32
#include <cassert>
#include <Windows.h>
#include <winternl.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/system/shared_library.h"

typedef struct _SECTIONBASICINFO
{
    PVOID         BaseAddress;
    ULONG         AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS
{
    SectionBasicInformation,
    SectionImageInformation
} SECTION_INFORMATION_CLASS,
    *PSECTION_INFORMATION_CLASS;

typedef NTSTATUS(NTAPI* NtQuerySection)(
    _In_ HANDLE SectionHandle, _In_ SECTION_INFORMATION_CLASS SectionInformationClass,
    _Out_writes_bytes_(SectionInformationLength) PVOID SectionInformation, _In_ SIZE_T SectionInformationLength, _Out_opt_ PSIZE_T ReturnLength
);

namespace zeus
{
namespace
{
auto ntQuerySection = zeus::SharedLibrary::Load(std::string("ntdll.dll"))
                          .transform([](const zeus::SharedLibrary& library) { return library.GetTypeSymbol<NtQuerySection>("NtQuerySection"); })
                          .value_or(nullptr);
}

struct SharedMemoryImpl
{
    std::string name;
    uint64_t    size = 0;
    WinHandle   handle;
};

SharedMemory::SharedMemory() : _impl(std::make_unique<SharedMemoryImpl>())
{
}

SharedMemory::SharedMemory(SharedMemory&& other) noexcept : _impl(std::move(other._impl))
{
}

SharedMemory& zeus::SharedMemory::operator=(SharedMemory&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

SharedMemory::~SharedMemory()
{
    if (_impl)
    {
        _impl->handle.Close();
    }
}

std::string SharedMemory::Name() const
{
    return _impl->name;
}

uint64_t SharedMemory::Size() const
{
    return _impl->size;
}

zeus::expected<MemoryMapping, std::error_code> SharedMemory::Map(uint64_t size, uint64_t offset, bool readOnly)
{
    assert(_impl->handle);
    return MemoryMapping::Map(_impl->handle, size, offset, readOnly);
}

HANDLE SharedMemory::Handle() const
{
    return _impl->handle;
}

zeus::expected<SharedMemory, std::error_code> SharedMemory::OpenOrCreate(const std::string& name, uint64_t size, bool readOnly)
{
    auto                wname         = CharsetUtils::UTF8ToUnicode(name);
    SECURITY_ATTRIBUTES securittyAttr = {};
    securittyAttr.nLength             = sizeof(SECURITY_ATTRIBUTES);
    securittyAttr.bInheritHandle      = FALSE;
    SECURITY_DESCRIPTOR sd            = {};
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
    SetSecurityDescriptorGroup(&sd, nullptr, FALSE);
    SetSecurityDescriptorSacl(&sd, FALSE, nullptr, FALSE);
    securittyAttr.lpSecurityDescriptor = &sd;
    WinHandle handle                   = CreateFileMappingW(
        INVALID_HANDLE_VALUE, &securittyAttr, readOnly ? PAGE_READONLY : PAGE_READWRITE, 0, size, wname.empty() ? nullptr : wname.c_str()
    );
    if (!handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    SharedMemory memory;
    memory._impl->handle                   = std::move(handle);
    memory._impl->name                     = name;
    SECTION_BASIC_INFORMATION info         = {};
    SIZE_T                    returnLength = 0;
    if (ntQuerySection && 0 == ntQuerySection(memory._impl->handle, SectionBasicInformation, &info, sizeof(info), &returnLength))
    {
        memory._impl->size = info.MaximumSize.QuadPart;
    }
    else
    {
        memory._impl->size = size;
    }
    return std::move(memory);
}

zeus::expected<SharedMemory, std::error_code> SharedMemory::Open(const std::string& name, bool readOnly)
{
    auto      wname  = CharsetUtils::UTF8ToUnicode(name);
    WinHandle handle = OpenFileMappingW((readOnly ? FILE_MAP_READ : FILE_MAP_WRITE) | SECTION_QUERY, FALSE, wname.c_str());
    if (!handle)
    {
        return zeus::unexpected(GetLastSystemError());
    }

    SharedMemory memory;
    memory._impl->handle                   = std::move(handle);
    memory._impl->name                     = name;
    SECTION_BASIC_INFORMATION info         = {};
    SIZE_T                    returnLength = 0;
    if (ntQuerySection && 0 == ntQuerySection(memory._impl->handle, SectionBasicInformation, &info, sizeof(info), &returnLength))
    {
        memory._impl->size = info.MaximumSize.QuadPart;
    }
    return std::move(memory);
}

} // namespace zeus
#endif