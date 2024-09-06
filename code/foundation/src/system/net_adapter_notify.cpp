#include "zeus/foundation/system/net_adapter_notify.h"

namespace zeus
{
std::string NetAdapterNotify::StateName(ConnectState state)
{
    switch (state)
    {
    case ConnectState::kDisconnected:
        return "Disconnected";
    case ConnectState::kConnected:
        return "Connected";
#ifdef _WIN32
    case ConnectState::kConnectedInternet:
        return "ConnectedInternet";
#endif
    default:
        return "";
    }
}
} // namespace zeus
