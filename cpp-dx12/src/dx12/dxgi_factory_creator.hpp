#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"

namespace dx12 {

class DxgiFactoryCreator final
{
public:
    DxgiFactoryCreator& EnableDebugLayer() noexcept
    {
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        ComPtr<ID3D12Debug> debug_controller;
        HRESULT hr = ::D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        if (SUCCEEDED(hr))
        {
            debug_controller->EnableDebugLayer();

            // Enable additional debug layers.
            flags_ |= DXGI_CREATE_FACTORY_DEBUG;
        }
        return *this;
    }

    DxgiFactoryCreator& EnableDebugLayerIf(bool enable) noexcept {
        return enable ? EnableDebugLayer() : *this;
    }

    DxgiFactoryCreator& EnableDebugLayerIfOnDebug() noexcept
    {
        bool enable_debug_layer = false;
#if defined(_DEBUG)
        enable_debug_layer = true;
#endif
        return EnableDebugLayerIf(enable_debug_layer);
    }

    /**
     * @exception DXException
     */
    ComPtr<IDXGIFactory4> Create() const
    {
        ComPtr<IDXGIFactory4> factory;
        HRESULT hr = ::CreateDXGIFactory2(flags_, IID_PPV_ARGS(&factory));
        DX_THROW_IF_FAILED(hr, "CreateDXGIFactory2");
        return factory;
    }

private:
    UINT flags_ = 0;
};

} // namespace dx12
