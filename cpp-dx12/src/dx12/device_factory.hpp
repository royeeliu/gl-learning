#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"

namespace dx12 {

class D3D12DeviceFactory final
{
public:
    D3D12DeviceFactory(IDXGIFactory4* dxgi_factory) noexcept
        : dxgi_factory_{dxgi_factory}
    {
        REQUIRE(dxgi_factory_);
    }

    D3D12DeviceFactory& UseWarpAdapter(bool use = true) noexcept
    {
        use_warp_adapter_ = use;
        return *this;
    }

    /**
     * @exception DXException
     */
    ComPtr<ID3D12Device> Create() const
    {
        auto adapter = GetAdapter();

        ComPtr<ID3D12Device> device;
        HRESULT hr = ::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        DX_THROW_IF_FAILED(hr, "D3D12CreateDevice");
        return device;
    }

private:
    /**
     * @exception DXException
     */
    ComPtr<IDXGIAdapter> GetAdapter() const
    {
        return use_warp_adapter_ ? GetWarpAdapter() : GetHardwareAdapter();
    }

    /**
     * @exception DXException
     */
    ComPtr<IDXGIAdapter> GetWarpAdapter() const 
    {
        ComPtr<IDXGIAdapter> adapter;
        HRESULT hr = dxgi_factory_->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
        DX_THROW_IF_FAILED(hr, "EnumWarpAdapter");
        return adapter;
    }

    /**
     * @exception DXException
     */
    ComPtr<IDXGIAdapter> GetHardwareAdapter() const
    {
        auto adapter = GetAdapterBySpecifiedPerformance();
        if (adapter)
        {
            return {ComPtr<IDXGIAdapter>(adapter.Get())};
        }

        std::optional<DXError> error;

        for (UINT index = 0;; ++index)
        {
            HRESULT hr = dxgi_factory_->EnumAdapters1(index, &adapter);
            if (FAILED(hr))
            {
                if (!error)
                {
                    error = MakeDXError(hr, "EnumAdapters1");
                }
                break;
            }

            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            hr = ::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(hr))
            {
                break;
            }
            error = MakeDXError(hr, "D3D12CreateDevice");
        }

        if (adapter)
        {
            return ComPtr<IDXGIAdapter>(adapter.Get());
        }

        ENSURE(error.has_value());
        ThrowException(std::move(* error));
    }

    ComPtr<IDXGIAdapter1> GetAdapterBySpecifiedPerformance() const noexcept {

        ComPtr<IDXGIFactory6> dxgi_factory6;
        HRESULT hr = dxgi_factory_->QueryInterface(IID_PPV_ARGS(&dxgi_factory6));
        if (FAILED(hr))
        {
            return nullptr;
        }

        ComPtr<IDXGIAdapter1> adapter;
        auto preference =
            request_high_performance_adapter_ ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
        for (UINT index = 0;; ++index)
        {
            hr = dxgi_factory6->EnumAdapterByGpuPreference(index, preference, IID_PPV_ARGS(&adapter));
            if (FAILED(hr))
            {
                break;
            }

            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            hr = ::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(hr))
            {
                break;
            }
        }

        return adapter;
    }

private:
    ComPtr<IDXGIFactory4> dxgi_factory_;
    bool use_warp_adapter_ = false;
    bool request_high_performance_adapter_ = false;
};

} // namespace dx12
