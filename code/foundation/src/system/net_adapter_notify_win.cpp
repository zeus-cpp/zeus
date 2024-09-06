#include "zeus/foundation/system/net_adapter_notify.h"
#ifdef _WIN32
#include <memory>
#include <map>
#include <thread>
#include <cassert>
#include <future>
#include <atlbase.h>
#include <netlistmgr.h>
#include "zeus/foundation/sync/event.h"
#include "zeus/foundation/sync/mutex_object.hpp"
#include "zeus/foundation/crypt/uuid.h"
#include "zeus/foundation/resource/win/com_init.h"
#include "zeus/foundation/container/callback_manager.hpp"

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "setupapi.lib")

namespace zeus
{
struct NetAdapterNotifyImpl
{
    std::unique_ptr<ComInit>                                            comInit;
    std::thread::id                                                     threadId;
    INetworkListManager*                                                manager                  = nullptr;
    IConnectionPointContainer*                                          connectionPointContainer = nullptr;
    IConnectionPoint*                                                   connectionPoint          = nullptr;
    unsigned long                                                       cookie                   = 0;
    std::thread                                                         thread;
    zeus::Event                                                         event;
    std::map<zeus::Uuid, NetAdapterNotify::ConnectState>                cache;
    CallbackManager<const std::string&, NetAdapterNotify::ConnectState> callbacks;
    std::mutex                                                          mutex;
    bool                                                                run = false;
};

namespace
{
NetAdapterNotify::ConnectState CastState(NLM_CONNECTIVITY state)
{
    if (state & NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV4_INTERNET)
    {
        return NetAdapterNotify::ConnectState::kConnectedInternet;
    }
    else if (state & NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV4_NOTRAFFIC)
    {
        return NetAdapterNotify::ConnectState::kConnected;
    }
    else
    {
        return NetAdapterNotify::ConnectState::kDisconnected;
    }
}

std::map<zeus::Uuid, NetAdapterNotify::ConnectState> GetNetAdapterStates(INetworkListManager* manager)
{
    std::map<zeus::Uuid, NetAdapterNotify::ConnectState> result;
    CComPtr<IEnumNetworkConnections>                     enumConnectons;
    if (SUCCEEDED(manager->GetNetworkConnections(&enumConnectons)))
    {
        while (enumConnectons)
        {
            CComPtr<INetworkConnection> connection;
            HRESULT                     hr = enumConnectons->Next(1, &connection, nullptr);
            if (SUCCEEDED(hr) && connection)
            {
                GUID adpaterIuid;
                hr = connection->GetAdapterId(&adpaterIuid);
                if (!FAILED(hr))
                {
                    NLM_CONNECTIVITY state = NLM_CONNECTIVITY::NLM_CONNECTIVITY_DISCONNECTED;
                    connection->GetConnectivity(&state);
                    result.emplace(Uuid::FromGuid(adpaterIuid), CastState(state));
                }
            }
            else
            {
                break;
            }
        }
    }
    return result;
}

class NetworkEvent : public INetworkConnectionEvents
{
public:
    NetworkEvent(NetAdapterNotifyImpl& impl) : _impl(impl), _count(1) {}

    virtual ~NetworkEvent() {}
    NetworkEvent(const NetworkEvent&)            = delete;
    NetworkEvent& operator=(const NetworkEvent&) = delete;
    NetworkEvent& operator=(NetworkEvent&&)      = delete;

    HRESULT STDMETHODCALLTYPE NetworkConnectionConnectivityChanged(GUID /*connectionId*/, NLM_CONNECTIVITY /*newConnectivity*/) override
    {
        std::map<zeus::Uuid, NetAdapterNotify::ConnectState> newChahe = GetNetAdapterStates(_impl.manager);

        for (auto& iter : newChahe)
        {
            auto old = _impl.cache.find(iter.first);
            if (old == _impl.cache.end())
            {
                _impl.callbacks.Call(iter.first.toString(), iter.second);
            }
            else
            {
                if (old->second != iter.second)
                {
                    _impl.callbacks.Call(iter.first.toString(), iter.second);
                }
            }
        }
        for (auto& iter : _impl.cache)
        {
            if (newChahe.find(iter.first) == newChahe.end())
            {
                _impl.callbacks.Call(iter.first.toString(), NetAdapterNotify::ConnectState::kDisconnected);
            }
        }
        _impl.cache.swap(newChahe);

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE NetworkConnectionPropertyChanged(
        /* [in] */ GUID /*connectionId*/,
        /* [in] */ NLM_CONNECTION_PROPERTY_CHANGE /*flags*/
    ) override
    {
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID                                               riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR* __RPC_FAR* ppvObject
    ) override
    {
        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        else if (IsEqualIID(riid, IID_INetworkConnectionEvents))
        {
            *ppvObject = dynamic_cast<INetworkConnectionEvents*>(this);
            AddRef();
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        MUTEX_OBJECT_LOCK(_count);
        _count.Value()++;
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        size_t count = 0;
        {
            MUTEX_OBJECT_LOCK(_count);
            count = --_count.Value();
        }
        if (0 == count)
        {
            delete this;
        }
        return S_OK;
    }
private:
    NetAdapterNotifyImpl&     _impl;
    zeus::MutexObject<size_t> _count;
};

bool StartNetManagerListen(NetAdapterNotifyImpl& impl)
{
    impl.comInit = std::make_unique<ComInit>(ComInit::ComType::MULTITHREADED);
    do
    {
        HRESULT hr = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_INetworkListManager, reinterpret_cast<LPVOID*>(&impl.manager));
        if (FAILED(hr))
        {
            break;
        }
        hr = impl.manager->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<LPVOID*>(&impl.connectionPointContainer));
        if (FAILED(hr))
        {
            break;
        }
        hr = impl.connectionPointContainer->FindConnectionPoint(IID_INetworkConnectionEvents, &impl.connectionPoint);
        if (FAILED(hr))
        {
            break;
        }
        impl.cache  = GetNetAdapterStates(impl.manager);
        auto* event = new NetworkEvent(impl);
        hr          = impl.connectionPoint->Advise(event, &impl.cookie);
        event->Release();
        if (FAILED(hr))
        {
            break;
        }
        return true;
    }
    while (false);
    return false;
}

void StopNetManagerListen(NetAdapterNotifyImpl& impl)
{
    if (impl.connectionPoint)
    {
        if (impl.cookie)
        {
            impl.connectionPoint->Unadvise(impl.cookie);
        }
        impl.connectionPoint->Release();
        impl.connectionPoint = nullptr;
    }
    if (impl.connectionPointContainer)
    {
        impl.connectionPointContainer->Release();
        impl.connectionPointContainer = nullptr;
    }
    if (impl.manager)
    {
        impl.manager->Release();
        impl.manager = nullptr;
    }
    impl.comInit.reset();
}

void RunNetManagerListen(NetAdapterNotifyImpl& impl, std::shared_ptr<std::promise<bool>> promise)
{
    const bool result = StartNetManagerListen(impl);
    promise->set_value(result);
    if (result)
    {
        impl.event.Wait();
    }
    StopNetManagerListen(impl);
}
} // namespace

NetAdapterNotify::NetAdapterNotify() : _impl(std::make_unique<NetAdapterNotifyImpl>())
{
}

NetAdapterNotify::NetAdapterNotify(NetAdapterNotify&& other) noexcept : _impl(std::move(other._impl))

{
}

NetAdapterNotify& NetAdapterNotify::operator=(NetAdapterNotify&& other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

NetAdapterNotify::~NetAdapterNotify()
{
    if (_impl)
    {
        assert(!_impl->thread.joinable());
        assert(!_impl->run);
    }
}

bool NetAdapterNotify::Start()
{
    std::lock_guard lock(_impl->mutex);
    if (_impl->run)
    {
        return false;
    }
    _impl->run = true;
    _impl->event.Reset();
    if (ComInit::ComType::APARTMENTTHREADED == ComInit::GetCurrentComType())
    {
        //advise的回调通知在单线程套件中必须通过本线程的窗口消息循环来分发，但这会阻塞此函数调用，这个不可接受的，所以此时只能新启动线程来完成初始化的操作
        auto promise      = std::make_shared<std::promise<bool>>();
        _impl->thread     = std::thread(std::bind(&RunNetManagerListen, std::ref(*_impl), promise));
        const auto result = promise->get_future().get();
        if (!result)
        {
            _impl->run = false;
            _impl->event.Notify();
            _impl->thread.join();
        }
        return result;
    }
    else
    {
        return StartNetManagerListen(*_impl);
    }
}
void NetAdapterNotify::Stop()
{
    std::lock_guard lock(_impl->mutex);
    if (_impl->run && _impl->thread.joinable())
    {
        _impl->event.Notify();
        _impl->thread.join();
    }
    _impl->run = false;
    _impl->event.Reset();
}
size_t NetAdapterNotify::AddConnectChangeCallback(
    const std::function<void(const std::string& adapterId, NetAdapterNotify::ConnectState state)>& callback
)
{
    return _impl->callbacks.AddCallback(callback);
}
bool NetAdapterNotify::RemoveConnectChangeCallback(size_t callbackId, bool wait)
{
    return _impl->callbacks.RemoveCallback(callbackId, wait);
}
} // namespace zeus
#endif