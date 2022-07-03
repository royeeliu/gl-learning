#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

namespace dx12 {

class DxgiSwapChainFactory final
{
public:
    static constexpr uint32_t DefaultBufferCount = 2;
    static constexpr DXGI_FORMAT DefaultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    DxgiSwapChainFactory(IDXGIFactory4* dxgi_factory, IUnknown* device) noexcept
        : dxgi_factory_{dxgi_factory}
        , device_{device}
    {
        REQUIRE(dxgi_factory_);
        REQUIRE(device_);
        desc_.BufferCount = DefaultBufferCount;
        desc_.Format = DefaultFormat;
        desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc_.SampleDesc.Count = 1;
    }

    DxgiSwapChainFactory& BufferCount(uint32_t buffer_count) noexcept
    {
        desc_.BufferCount = buffer_count;
        return *this;
    }

    ComPtr<IDXGISwapChain1> CreateForHwnd(HWND hwnd) const 
    {
        ASSERT(::IsWindow(hwnd));
        auto desc = desc_;
        if ((desc.Width == 0) || (desc.Height == 0))
        {
            RECT rect{};
            BOOL br = ::GetClientRect(hwnd, &rect);
            ASSERT(br);
            desc.Width = rect.right - rect.left;
            desc.Height = rect.bottom - rect.top;
        }

        ComPtr<IDXGISwapChain1> swap_chain;
        HRESULT hr = dxgi_factory_->CreateSwapChainForHwnd(device_.Get(), hwnd, &desc, nullptr, nullptr, &swap_chain);
        DX_THROW_IF_FAILED(hr, "CreateSwapChainForHwnd");
        return swap_chain;
    }

private:
    ComPtr<IDXGIFactory4> dxgi_factory_;
    ComPtr<IUnknown> device_;
    DXGI_SWAP_CHAIN_DESC1 desc_{};
};

} // namespace dx12
