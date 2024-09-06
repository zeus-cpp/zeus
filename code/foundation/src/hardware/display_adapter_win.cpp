#include "zeus/foundation/hardware/display_adapter.h"

#ifdef _WIN32
#include <set>
#include <DXGI.h>
#include <atlbase.h>
#include "zeus/foundation/string/charset_utils.h"
#include "impl/display_adapter_impl.h"
#pragma comment(lib, "DXGI.lib")

namespace zeus::Hardware
{

std::vector<DisplayAdapter> DisplayAdapter::ListAll()
{
    std::vector<DisplayAdapter> result;
    CComPtr<IDXGIFactory>       factory;
    HRESULT                     hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
    if (SUCCEEDED(hr))
    {
        std::set<UINT> deviceIds;

        for (UINT index = 0;; ++index)
        {
            CComPtr<IDXGIAdapter> adpater;
            if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters(index, &adpater))
            {
                break;
            }
            DXGI_ADAPTER_DESC adapterDesc;
            hr = adpater->GetDesc(&adapterDesc);
            if (SUCCEEDED(hr))
            {
                if (std::wstring(L"Microsoft Basic Render Driver") == std::wstring(adapterDesc.Description))
                {
                    continue;
                }
                if (deviceIds.count(adapterDesc.DeviceId))
                {
                    continue;
                }
                DisplayAdapter item;
                item._impl->describeName = CharsetUtils::UnicodeToUTF8(adapterDesc.Description);
                item._impl->vendorId     = adapterDesc.VendorId;
                item._impl->sharedMemory = adapterDesc.SharedSystemMemory / 1024;
                item._impl->videoMemory  = adapterDesc.DedicatedVideoMemory / 1024;
                item._impl->systemMemory = adapterDesc.DedicatedSystemMemory / 1024;
                deviceIds.emplace(adapterDesc.DeviceId);
                result.emplace_back(std::move(item));
            }
        }
    }
    return result;
}
} // namespace zeus::Hardware
#endif
