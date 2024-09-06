#include "zeus/foundation/file/file_wrapper.h"
#ifdef __linux__
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/time/time_utils.h"

namespace zeus
{

namespace
{
int OffsetTypeToWhence(FileWrapper::OffsetType type)
{
    switch (type)
    {
    case FileWrapper::OffsetType::kBegin:
        return SEEK_SET;
    case FileWrapper::OffsetType::kCurrent:
        return SEEK_CUR;
    case FileWrapper::OffsetType::kEnd:
        return SEEK_END;
    default:
        assert(false);
        return SEEK_SET;
    }
}

int ModeToFlag(FileWrapper::OpenMode mode)
{
    switch (mode)
    {
    case FileWrapper::OpenMode::kRead:
        return O_RDONLY;
    case FileWrapper::OpenMode::kWrite:
        return O_WRONLY;
    case FileWrapper::OpenMode::kReadWrite:
        return O_RDWR;
    default:
        assert(false);
        return O_RDWR;
    }
}

zeus::expected<FileWrapper, std::error_code> OpenWrapper(const std::filesystem::path& path, FileWrapper::OpenMode mode, int flag, bool autoFlush)
{
    if (autoFlush)
    {
        flag |= O_SYNC;
    }
    int fd = open(path.c_str(), ModeToFlag(mode) | flag, S_IRWXU | S_IRWXG | S_IRWXO);
    if (-1 == fd)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return FileWrapper(fd);
}

} // namespace

struct FileWrapperImpl
{
    LinuxFileDescriptor fileDescriptor;
};

FileWrapper::FileWrapper() : _impl(std::make_unique<FileWrapperImpl>())
{
}

FileWrapper::FileWrapper(PlatformFileHandle handle) : _impl(std::make_unique<FileWrapperImpl>(FileWrapperImpl {handle}))
{
}

FileWrapper::FileWrapper(PlatformFileHandleWrapper&& handle) : _impl(std::make_unique<FileWrapperImpl>())
{
    _impl->fileDescriptor = std::move(handle);
}
FileWrapper& FileWrapper::operator=(PlatformFileHandleWrapper&& handle) noexcept
{
    _impl->fileDescriptor = std::move(handle);
    return *this;
}
FileWrapper::FileWrapper(FileWrapper&& other) noexcept : _impl(std::move(other._impl))
{
}

FileWrapper& FileWrapper::operator=(FileWrapper&& other) noexcept
{
    if (this != std::addressof(other))
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
FileWrapper::~FileWrapper()
{
}

FileWrapper::operator int() const noexcept
{
    return _impl->fileDescriptor.operator int();
}

FileWrapper::operator int*() const noexcept
{
    return _impl->fileDescriptor.operator int*();
}

FileWrapper::operator bool() const noexcept
{
    return !Empty();
}
bool FileWrapper::operator==(const PlatformFileHandleWrapper& handle) const noexcept
{
    return _impl->fileDescriptor == handle;
}
bool FileWrapper::operator!=(const PlatformFileHandleWrapper& handle) const noexcept
{
    return _impl->fileDescriptor != handle;
}
bool FileWrapper::operator==(PlatformFileHandle handle) const noexcept
{
    return _impl->fileDescriptor == handle;
}
bool FileWrapper::operator!=(PlatformFileHandle handle) const noexcept
{
    return _impl->fileDescriptor != handle;
}

bool FileWrapper::operator==(const FileWrapper& other) const noexcept
{
    return _impl->fileDescriptor == other._impl->fileDescriptor;
}
bool FileWrapper::operator!=(const FileWrapper& other) const noexcept
{
    return _impl->fileDescriptor != other._impl->fileDescriptor;
}

void FileWrapper::Close() noexcept
{
    _impl->fileDescriptor.Close();
}
bool FileWrapper::Empty() const noexcept
{
    return _impl->fileDescriptor.Empty();
}
void FileWrapper::Attach(PlatformFileHandle handle) noexcept
{
    _impl->fileDescriptor.Attach(handle);
}
PlatformFileHandle FileWrapper::Detach() noexcept
{
    return _impl->fileDescriptor.Detach();
}

PlatformFileHandle FileWrapper::PlatformHandle() const noexcept
{
    return _impl->fileDescriptor.FileDescriptor();
}

int FileWrapper::Fd() const noexcept
{
    return _impl->fileDescriptor.Fd();
}

int FileWrapper::FileDescriptor() const noexcept
{
    return _impl->fileDescriptor.FileDescriptor();
}

zeus::expected<LinuxFileDescriptor, std::error_code> FileWrapper::Duplicate(bool closeOnExec) const
{
    return _impl->fileDescriptor.Duplicate(closeOnExec);
}

zeus::expected<bool, std::error_code> FileWrapper::IsCloseOnExec() const
{
    return _impl->fileDescriptor.IsCloseOnExec();
}

zeus::expected<void, std::error_code> FileWrapper::SetCloseOnExec(bool closeOnExec)
{
    return _impl->fileDescriptor.SetCloseOnExec(closeOnExec);
}

zeus::expected<uint64_t, std::error_code> FileWrapper::FileSize()
{
    assert(!Empty());
    struct stat statbuf
    {
    };
    if (fstat(_impl->fileDescriptor.FileDescriptor(), &statbuf) == -1)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return statbuf.st_size;
}
zeus::expected<size_t, std::error_code> FileWrapper::Write(const void* data, size_t size)
{
    assert(!Empty());
    ssize_t written = write(_impl->fileDescriptor.FileDescriptor(), data, size);
    if (written == -1)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return written;
}

zeus::expected<size_t, std::error_code> FileWrapper::Read(void* buffer, size_t size)
{
    assert(!Empty());
    ssize_t readSize = read(_impl->fileDescriptor.FileDescriptor(), buffer, size);
    if (readSize == -1)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return readSize;
}
zeus::expected<uint64_t, std::error_code> FileWrapper::Seek(int64_t offset, OffsetType type)
{
    assert(!Empty());
    off_t pos = lseek(_impl->fileDescriptor.FileDescriptor(), offset, OffsetTypeToWhence(type));
    if (pos == -1)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return pos;
}
zeus::expected<uint64_t, std::error_code> FileWrapper::Tell()
{
    assert(!Empty());
    off_t offset = lseek(_impl->fileDescriptor.FileDescriptor(), 0, SEEK_CUR);
    if (offset == -1)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return offset;
}
zeus::expected<void, std::error_code> FileWrapper::Flush()
{
    assert(!Empty());
    if (-1 == fsync(_impl->fileDescriptor.FileDescriptor()))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return {};
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> FileWrapper::CreateTime()
{
    assert(!Empty());
    struct statx statxBuf
    {
    };
    if (-1 == statx(_impl->fileDescriptor.FileDescriptor(), "", AT_EMPTY_PATH, STATX_BTIME, &statxBuf))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return std::chrono::system_clock::from_time_t(statxBuf.stx_btime.tv_sec);
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> FileWrapper::LastAccessTime()
{
    assert(!Empty());
    struct stat statbuf
    {
    };
    if (-1 == fstat(_impl->fileDescriptor.FileDescriptor(), &statbuf))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return std::chrono::system_clock::from_time_t(statbuf.st_atime);
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> FileWrapper::LastWriteTime()
{
    assert(!Empty());
    struct stat statbuf
    {
    };
    if (-1 == fstat(_impl->fileDescriptor.FileDescriptor(), &statbuf))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return std::chrono::system_clock::from_time_t(statbuf.st_mtime);
}

zeus::expected<std::chrono::system_clock::time_point, std::error_code> zeus::FileWrapper::LastChangeTime()
{
    assert(!Empty());
    struct stat statbuf
    {
    };
    if (-1 == fstat(_impl->fileDescriptor.FileDescriptor(), &statbuf))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    return std::chrono::system_clock::from_time_t(statbuf.st_ctime);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::Open(const std::filesystem::path& path, OpenMode mode)
{
    return OpenWrapper(path, mode, O_CLOEXEC, false);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::Create(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return OpenWrapper(path, mode, O_CREAT | O_EXCL | O_CLOEXEC, autoFlush);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::OpenOrCreate(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return OpenWrapper(path, mode, O_CREAT | O_CLOEXEC, autoFlush);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::Truncate(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return OpenWrapper(path, mode, O_CREAT | O_TRUNC | O_CLOEXEC, autoFlush);
}

zeus::expected<FileWrapper, std::error_code> FileWrapper::OpenSymbolLink(const std::filesystem::path& path, OpenMode mode, bool autoFlush)
{
    return OpenWrapper(path, mode, O_NOFOLLOW | O_PATH | O_CLOEXEC, autoFlush);
}
} // namespace zeus
#endif