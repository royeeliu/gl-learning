#include "sample_devices.h"
#include "dx12/dx12_factories.hpp"

namespace samples {

void SampleDevices::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{
    try
    {
        auto dxgi_factory = dx12::DxgiFactoryCreator().EnableDebugLayerIfOnDebug().Create();
        device = dx12::D3D12DeviceFactory(dxgi_factory.Get()).Create();
        std::tie(command_queue, command_allocator) = dx12::D3D12CommandQueueFactory(device.Get()).Create();

        // Swap chain needs the queue so that it can force a flush on it.
        auto swap_chain1 = dx12::DxgiSwapChainFactory(dxgi_factory.Get(), command_queue.Get())
                               .BufferCount(SWAP_CHAIN_BUFFER_COUNT)
                               .Size(width, height)
                               .CreateForHwnd(hwnd);

        // This sample does not support fullscreen transitions.
        HRESULT hr = dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        DX_THROW_IF_FAILED(hr, "MakeWindowAssociation");

        hr = swap_chain1.As(&swap_chain);
        DX_THROW_IF_FAILED(hr, "As");

        // Describe and create a render target view (RTV) descriptor heap.
        rtv_heap = dx12::D3D12DescriptorHeapFactory(device.Get())
                       .Type(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
                       .NumDescriptors(SWAP_CHAIN_BUFFER_COUNT)
                       .Create();
        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create frame resources.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++)
        {
            hr = swap_chain->GetBuffer(n, IID_PPV_ARGS(&render_targets[n]));
            DX_THROW_IF_FAILED(hr, "GetBuffer");
            device->CreateRenderTargetView(render_targets[n].Get(), nullptr, rtv_handle);
            rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }
    catch (base::Exception&)
    {
        *this = {};
        throw;
    }
}

} // namespace samples
