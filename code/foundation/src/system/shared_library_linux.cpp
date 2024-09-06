#include "zeus/foundation/system/shared_library.h"

#ifdef __linux__
#include <dlfcn.h>

namespace zeus
{
struct SharedLibraryImpl
{
    void* module = nullptr;
};

SharedLibrary::SharedLibrary(void* module) : _impl(std::make_unique<SharedLibraryImpl>())
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
        dlclose(_impl->module);
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
    return dlsym(_impl->module, name.c_str());
}

void* SharedLibrary::Module() const noexcept
{
    return _impl->module;
}

SharedLibrary::operator void*() const noexcept
{
    return _impl->module;
}

zeus::expected<SharedLibrary, SharedLibrary::ErrorType> SharedLibrary::Load(const std::filesystem::path& path)
{
    return Load(path, RTLD_NOW);
}
zeus::expected<SharedLibrary, SharedLibrary::ErrorType> SharedLibrary::Load(const std::filesystem::path& path, uint32_t flag)
{
    void* module = dlopen(path.c_str(), flag);
    if (module)
    {
        return SharedLibrary(module);
    }
    return zeus::unexpected(dlerror());
}

std::string SharedLibrary::Extension()
{
    return ".so";
}
} // namespace zeus
#endif
