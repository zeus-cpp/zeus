#ifdef _WIN32
#include "zeus/foundation/system/shared_library.h"

#include <Windows.h>

#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/string/string_utils.h"

namespace zeus
{
struct SharedLibraryImpl
{
    HMODULE module = nullptr;
};

SharedLibrary::SharedLibrary(const HMODULE& module) : _impl(std::make_unique<SharedLibraryImpl>())
{
    _impl->module = module;
}

SharedLibrary::~SharedLibrary()
{
    Unload();
}

SharedLibrary::SharedLibrary(SharedLibrary&& other) noexcept : _impl(std::make_unique<SharedLibraryImpl>())
{
    std::swap(_impl, other._impl);
}

SharedLibrary& SharedLibrary::operator=(SharedLibrary&& other) noexcept
{
    Unload();
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

void SharedLibrary::Unload() noexcept
{
    if (_impl->module)
    {
        FreeLibrary(_impl->module);
    }
    _impl->module = nullptr;
}

bool SharedLibrary::Empty() const noexcept
{
    return !_impl->module;
}

void* SharedLibrary::GetSymbol(const std::string& name) const noexcept
{
    if (Empty())
    {
        return nullptr;
    }
    return GetProcAddress(_impl->module, name.c_str());
}

HMODULE SharedLibrary::Module() const noexcept
{
    return _impl->module;
}

SharedLibrary::operator HMODULE() const noexcept
{
    return _impl->module;
}

zeus::expected<SharedLibrary, SharedLibrary::ErrorType> SharedLibrary::Load(const std::filesystem::path& path)
{
    return Load(path, 0);
}

zeus::expected<SharedLibrary, SharedLibrary::ErrorType> SharedLibrary::Load(const std::filesystem::path& path, uint32_t flag)
{
    auto filepath = path.wstring();
    if (StartWith(filepath, LR"(\??\)"))
    {
        filepath = filepath.substr(4);
    }
    HMODULE module = LoadLibraryExW(filepath.c_str(), nullptr, flag);
    if (module)
    {
        return SharedLibrary(module);
    }
    return zeus::unexpected(GetLastSystemError());
}

std::string SharedLibrary::Extension()
{
    return ".dll";
}
} // namespace zeus
#endif
