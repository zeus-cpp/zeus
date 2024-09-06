#include "zeus/foundation/sync/linux/file_descriptor_set_notify.h"
#ifdef __linux__
#include <map>
#include <mutex>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <utility>
#include <cassert>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include "zeus/foundation/core/posix/eintr_wrapper.h"
#include "zeus/foundation/resource/linux/file_descriptor.h"
#include "zeus/foundation/container/callback_manager.hpp"
#include "zeus/foundation/sync/event.h"

namespace zeus
{

class EpollFileDescriptorManager
{
public:
    void AddFileDescriptor(int epollFileDescriptor, int fileDescriptor, bool readable, bool writable)
    {
        assert(readable || writable);
        if (auto iter = _referenceCount.find(fileDescriptor); iter != _referenceCount.end())
        {
            auto& count  = iter->second;
            bool  change = false;
            if (readable)
            {
                change = change | (0 == count.read++);
            }
            if (writable)
            {
                change = change | (0 == count.write++);
            }
            if (change)
            {
                epoll_event event {};
                event.events  = EPOLLIN | EPOLLOUT;
                event.data.fd = fileDescriptor;
                epoll_ctl(epollFileDescriptor, EPOLL_CTL_MOD, fileDescriptor, &event);
            }
        }
        else
        {
            auto& count = _referenceCount[fileDescriptor];
            if (readable)
            {
                ++count.read;
            }
            if (writable)
            {
                ++count.write;
            }
            epoll_event event {};
            event.events  = (count.read ? EPOLLIN : 0) | (count.write ? EPOLLOUT : 0);
            event.data.fd = fileDescriptor;
            epoll_ctl(epollFileDescriptor, EPOLL_CTL_ADD, fileDescriptor, &event);
        }
    }
    void RemoveReadWriteFileDescriptor(int epollFileDescriptor, int fileDescriptor, bool readable, bool writable)
    {
        assert(readable || writable);
        auto iter = _referenceCount.find(fileDescriptor);
        assert(iter != _referenceCount.end());
        auto& count  = iter->second;
        bool  change = false;
        if (readable)
        {
            change = change | (0 == --count.read);
        }
        if (writable)
        {
            change = change | (0 == --count.write);
        }
        if (0 == count.read && 0 == count.write)
        {
            epoll_ctl(epollFileDescriptor, EPOLL_CTL_DEL, fileDescriptor, nullptr);
            _referenceCount.erase(iter);
        }
        else
        {
            epoll_event event {};
            event.events  = (count.read ? EPOLLIN : 0) | (count.write ? EPOLLOUT : 0);
            event.data.fd = fileDescriptor;
            epoll_ctl(epollFileDescriptor, EPOLL_CTL_MOD, fileDescriptor, &event);
        }
    }
private:
    struct Count
    {
        size_t read  = 0;
        size_t write = 0;
    };
    std::map<int, Count> _referenceCount;
};

struct FileDescriptorState
{
    int  fileDescriptor;
    bool readable  = false;
    bool writeable = false;
};

struct LinuxFileDescriptorSetNotifyImpl
{
    bool                                  automatic = false;
    LinuxFileDescriptor                   wakeupFd;
    LinuxFileDescriptor                   epollFd;
    std::mutex                            mutex;
    std::thread                           thread;
    std::atomic<bool>                     run = false;
    std::map<size_t, FileDescriptorState> callbackFileDescriptor;
    EpollFileDescriptorManager            fileDescriptorManager;
    NameCallbackManager<int, bool, bool>  callbackManager;
};

void WaitEpollEvent(LinuxFileDescriptorSetNotifyImpl& impl)
{
    while (impl.run)
    {
        epoll_event event;
        int         nfds = epoll_wait(impl.epollFd.Fd(), &event, 1, -1);
        if (nfds > 0)
        {
            if (event.data.fd == impl.wakeupFd.Fd())
            {
                uint64_t value;
                HANDLE_EINTR(read(impl.wakeupFd.Fd(), &value, sizeof(value)));
                continue;
            }
            else
            {
                bool readable = event.events & EPOLLIN;
                bool writable = event.events & EPOLLOUT;
                impl.callbackManager.Call(event.data.fd, readable, writable);
            }
        }
    }
}
void WakeupEpoll(LinuxFileDescriptorSetNotifyImpl& impl)
{
    const uint64_t value = 1;
    HANDLE_EINTR(write(impl.wakeupFd.Fd(), &value, sizeof(value)));
}

LinuxFileDescriptorSetNotify::LinuxFileDescriptorSetNotify(bool automatic) : _impl(std::make_unique<LinuxFileDescriptorSetNotifyImpl>())
{
    _impl->automatic = automatic;
    _impl->epollFd   = epoll_create1(EPOLL_CLOEXEC);
    _impl->wakeupFd  = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(_impl->epollFd);
    assert(_impl->wakeupFd);
    epoll_event wake {};
    wake.events  = EPOLLIN;
    wake.data.fd = _impl->wakeupFd.Fd();
    epoll_ctl(_impl->epollFd.Fd(), EPOLL_CTL_ADD, _impl->wakeupFd.Fd(), &wake);
}

LinuxFileDescriptorSetNotify::~LinuxFileDescriptorSetNotify()
{
    if (_impl)
    {
        if (_impl->automatic)
        {
            Stop();
        }
        assert(!_impl->run);
        assert(!_impl->thread.joinable());
    }
}

size_t LinuxFileDescriptorSetNotify::AddFileDescriptorReadableStateCallback(int fileDescriptor, const std::function<void()>& callback)
{
    size_t id = 0;
    {
        std::unique_lock<std::mutex> lock(_impl->mutex);
        id = _impl->callbackManager.AddCallback(
            fileDescriptor,
            [callback](bool readable, bool /* writable */)
            {
                if (readable)
                {
                    callback();
                }
            }
        );
        _impl->callbackFileDescriptor.emplace(id, FileDescriptorState {fileDescriptor, true, false});
        _impl->fileDescriptorManager.AddFileDescriptor(_impl->epollFd.FileDescriptor(), fileDescriptor, true, false);
    }
    if (_impl->automatic)
    {
        Start();
    }
    return id;
}
size_t LinuxFileDescriptorSetNotify::AddFileDescriptorWritableStateCallback(int fileDescriptor, const std::function<void()>& callback)
{
    size_t id = 0;
    {
        std::unique_lock<std::mutex> lock(_impl->mutex);
        id = _impl->callbackManager.AddCallback(
            fileDescriptor,
            [callback](bool /* readable */, bool writable)
            {
                if (writable)
                {
                    callback();
                }
            }
        );
        _impl->callbackFileDescriptor.emplace(id, FileDescriptorState {fileDescriptor, false, true});
        _impl->fileDescriptorManager.AddFileDescriptor(_impl->epollFd.FileDescriptor(), fileDescriptor, false, true);
    }
    if (_impl->automatic)
    {
        Start();
    }
    return id;
}
size_t LinuxFileDescriptorSetNotify::AddFileDescriptorStateCallback(
    int fileDescriptor, const std::function<void(bool readable, bool writable)>& callback
)
{
    size_t id = 0;
    {
        std::unique_lock<std::mutex> lock(_impl->mutex);
        id = _impl->callbackManager.AddCallback(fileDescriptor, callback);
        _impl->callbackFileDescriptor.emplace(id, FileDescriptorState {fileDescriptor, true, true});
        _impl->fileDescriptorManager.AddFileDescriptor(_impl->epollFd.FileDescriptor(), fileDescriptor, true, true);
    }
    if (_impl->automatic)
    {
        Start();
    }
    return id;
}

bool LinuxFileDescriptorSetNotify::RemoveFileDescriptorStateCallback(size_t callbackId, bool wait)
{
    std::unique_lock<std::mutex> lock(_impl->mutex);
    if (auto fileDescriptorIter = _impl->callbackFileDescriptor.find(callbackId); fileDescriptorIter != _impl->callbackFileDescriptor.end())
    {
        _impl->callbackManager.RemoveCallback(callbackId, wait);
        auto& fileDescriptorState = fileDescriptorIter->second;
        _impl->fileDescriptorManager.RemoveReadWriteFileDescriptor(
            _impl->epollFd.FileDescriptor(), fileDescriptorState.fileDescriptor, fileDescriptorState.readable, fileDescriptorState.writeable
        );
        _impl->callbackFileDescriptor.erase(fileDescriptorIter);
        return true;
    }
    else
    {
        return false;
    }
}

void LinuxFileDescriptorSetNotify::Start()
{
    std::unique_lock<std::mutex> lock(_impl->mutex);
    if (!_impl->run)
    {
        _impl->run    = true;
        _impl->thread = std::thread(WaitEpollEvent, std::ref(*_impl));
    }
}

void LinuxFileDescriptorSetNotify::Stop()
{
    std::unique_lock<std::mutex> lock(_impl->mutex);
    if (_impl->run)
    {
        _impl->run = false;
        WakeupEpoll(*_impl);
        if (_impl->thread.joinable())
        {
            _impl->thread.join();
        }
    }
}

} // namespace zeus
#endif