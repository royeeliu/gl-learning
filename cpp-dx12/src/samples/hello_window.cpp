#include "hello_window.h"
#include "dx12/d3dx12.hpp"
#include "dx12/dx12_factories.h"

#include <stdexcept>

using namespace dx12;

namespace samples {

HelloWindow::HelloWindow(HWND hwnd, ErrorCallback on_error) noexcept
    : hwnd_{hwnd}
    , on_error_{std::move(on_error)}
{
    try
    {
        LoadPipeline();
        LoadAssets();
    }
    catch (dx12::DXException& ex)
    {
        OnDXError(ex.Error());
    }
}

HelloWindow::~HelloWindow() noexcept
{
    try
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitForPreviousFrame();
    }
    catch (dx12::DXException& ex)
    {
        OnDXError(ex.Error());
    }

    fence_event_.reset();
}

void HelloWindow::Update() noexcept {}

bool HelloWindow::Render() noexcept
{
    if (error_status_)
    {
        return false;
    }

    try
    {
        DoRedner();
        return true;
    }
    catch (dx12::DXException& ex)
    {
        OnDXError(ex.Error());
        return false;
    }
}

void HelloWindow::LoadPipeline()
{
    auto dxgi_factory = dx12::DxgiFactoryCreator().EnableDebugLayerIfOnDebug().Create();
    auto device = dx12::D3D12DeviceFactory(dxgi_factory.Get()).UseWarpAdapter(use_warp_device_).Create();
    auto [command_queue, command_allocator] = dx12::D3D12CommandQueueFactory(device.Get()).Create();

    // Swap chain needs the queue so that it can force a flush on it.
    auto swap_chain1 = dx12::DxgiSwapChainFactory(dxgi_factory.Get(), command_queue.Get())
                           .BufferCount(FrameCount)
                           .CreateForHwnd(hwnd_);

    // This sample does not support fullscreen transitions.
    HRESULT hr = dxgi_factory->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);
    DX_THROW_IF_FAILED(hr, "MakeWindowAssociation");

    ComPtr<IDXGISwapChain3> swap_chain3;
    hr = swap_chain1.As(&swap_chain3);
    DX_THROW_IF_FAILED(hr, "As");

    // Describe and create a render target view (RTV) descriptor heap.
    auto rtv_heap = dx12::D3D12DescriptorHeapFactory(device.Get())
                        .Type(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
                        .NumDescriptors(FrameCount)
                        .Create();
    UINT rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create frame resources.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        hr = swap_chain3->GetBuffer(n, IID_PPV_ARGS(&render_targets_[n]));
        DX_THROW_IF_FAILED(hr, "GetBuffer");
        device->CreateRenderTargetView(render_targets_[n].Get(), nullptr, rtv_handle);
        rtv_handle.Offset(1, rtv_descriptor_size);
    }

    device_ = std::move(device);
    swap_chain_ = std::move(swap_chain3);
    command_queue_ = std::move(command_queue);
    command_allocator_ = std::move(command_allocator);
    rtv_heap_ = std::move(rtv_heap);
    rtv_descriptor_size_ = rtv_descriptor_size;
    frame_index_ = swap_chain_->GetCurrentBackBufferIndex();
}

void HelloWindow::LoadAssets()
{
    // Create the command list.
    HRESULT hr =
        device_->CreateCommandList(0, CommandListType, command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
    DX_THROW_IF_FAILED(hr, "CreateCommandList");

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    hr = command_list_->Close();
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");

    // Create synchronization objects.
    hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    fence_value_ = 1;

    // Create an event handle to use for frame synchronization.
    fence_event_.reset(::CreateEvent(nullptr, FALSE, FALSE, nullptr));
    if (fence_event_ == nullptr)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        DX_THROW_IF_FAILED(hr, "CreateEvent");
    }
}

void HelloWindow::DoRedner()
{
    REQUIRE(!error_status_);
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* command_lists[] = {command_list_.Get()};
    command_queue_->ExecuteCommandLists(_countof(command_lists), command_lists);

    // Present the frame.
    HRESULT hr = swap_chain_->Present(1, 0);
    DX_THROW_IF_FAILED(hr, "Present");

    WaitForPreviousFrame();
}

void HelloWindow::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    HRESULT hr = command_allocator_->Reset();
    DX_THROW_IF_FAILED(hr, "ID3D12CommandAllocator::Reset");

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    hr = command_list_->Reset(command_allocator_.Get(), pipeline_state_.Get());
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Reset");

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets_[frame_index_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    command_list_->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
        rtv_heap_->GetCPUDescriptorHandleForHeapStart(), frame_index_, rtv_descriptor_size_);

    // Record commands.
    const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    command_list_->ClearRenderTargetView(rtv_handle, clearColor, 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets_[frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    command_list_->ResourceBarrier(1, &barrier);

    hr = command_list_->Close();
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");
}

void HelloWindow::WaitForPreviousFrame()
{
    if (!command_queue_ || !fence_)
    {
        return;
    }

    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = fence_value_;
    HRESULT hr = command_queue_->Signal(fence_.Get(), fence);
    DX_THROW_IF_FAILED(hr, "Signal");
    fence_value_++;

    // Wait until the previous frame is finished.
    if (fence_->GetCompletedValue() < fence)
    {
        hr = fence_->SetEventOnCompletion(fence, fence_event_.get());
        DX_THROW_IF_FAILED(hr, "Signal");
        ::WaitForSingleObject(fence_event_.get(), INFINITE);
    }

    frame_index_ = swap_chain_->GetCurrentBackBufferIndex();
}

void HelloWindow::OnDXError(base::Error const& error) {
    error_status_ = true;

    if (on_error_)
    {
        on_error_(error);
    }
}

} // namespace samples
