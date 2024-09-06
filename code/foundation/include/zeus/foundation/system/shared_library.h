#pragma once

#include <memory>
#include <string>
#include <system_error>
#include <filesystem>
#include <zeus/expected.hpp>
#ifdef _WIN32
#include "zeus/foundation/core/win/win_windef.h"
#endif

namespace zeus
{
struct SharedLibraryImpl;
class SharedLibrary
{
public:

#ifdef _WIN32
    using ErrorType = std::error_code;
    SharedLibrary(const HMODULE& module);
#endif
#ifdef __linux__
    using ErrorType = std::string;
    SharedLibrary(void* module);
#endif
    ~SharedLibrary();
    SharedLibrary(const SharedLibrary&) = delete;
    SharedLibrary(SharedLibrary&& other) noexcept;
    SharedLibrary& operator=(const SharedLibrary&) = delete;
    SharedLibrary& operator=(SharedLibrary&& other) noexcept;
    explicit operator bool() const noexcept;

    void  Unload() noexcept;
    bool  Empty() const noexcept;
    bool  HasSymbol(const std::string& name) const noexcept;
    void* GetSymbol(const std::string& name) const noexcept;
    template<class Type>
    Type GetTypeSymbol(const std::string& name) const noexcept;
#ifdef _WIN32
    HMODULE Module() const noexcept;
    operator HMODULE() const noexcept;
#endif
#ifdef __linux__
    void* Module() const noexcept;
    operator void*() const noexcept;
#endif
public:
    static zeus::expected<SharedLibrary, ErrorType> Load(const std::filesystem::path& path);
    static zeus::expected<SharedLibrary, ErrorType> Load(const std::filesystem::path& path, uint32_t flag);
    static std::string                              Extension();
private:
    std::unique_ptr<SharedLibraryImpl> _impl;
};
template<typename Type>
inline Type SharedLibrary::GetTypeSymbol(const std::string& name) const noexcept
{
    return reinterpret_cast<Type>(GetSymbol(name));
}
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
