#pragma once
#include "zeus/foundation/core/win/win_windef.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/resource/win/handle.h"
namespace zeus
{
#ifdef _WIN32
using PlatformFileHandle          = HANDLE;
using PlatformFileHandleWrapper   = WinHandle;
using PlatformMemoryMappingHandle = HANDLE;
#endif
#ifdef __linux__
using PlatformFileHandle          = int;
using PlatformFileHandleWrapper   = LinuxFileDescriptor;
using PlatformMemoryMappingHandle = int;
#endif
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
