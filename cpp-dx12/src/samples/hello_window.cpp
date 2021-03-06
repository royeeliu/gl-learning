#include "hello_window.h"
#include "dx12/d3dx12.hpp"
#include "dx12/dx12_factories.hpp"
#include "dx12/dx12_utils.hpp"

namespace samples {

HelloWindow::HelloWindow(HWND hwnd, ErrorCallback on_error) noexcept
    : hwnd_{hwnd}
    , on_error_{std::move(on_error)}
{
    RECT rect{};
    ::GetClientRect(hwnd_, &rect);
    width_ = rect.right - rect.left;
    height_ = rect.bottom - rect.top;

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
    devices_.Initialize(hwnd_, width_, height_);
    frame_index_ = devices_.swap_chain->GetCurrentBackBufferIndex();
}

void HelloWindow::LoadAssets()
{
    auto& device = devices_.device;
    auto& command_allocator = devices_.command_allocator;
    
    // Create the command list.
    command_list_ = dx12::D3D12CommandListFactory(device.Get(), command_allocator.Get()).Create();

    // Create synchronization objects.
    std::tie(fence_, fence_event_) = dx12::D3D12FenceFactory(device.Get()).Create();
    fence_value_ = 1;
}

void HelloWindow::DoRedner()
{
    REQUIRE(!error_status_);
    auto& command_queue = devices_.command_queue;
    auto& swap_chain = devices_.swap_chain;

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* command_lists[] = {command_list_.Get()};
    command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

    // Present the frame.
    HRESULT hr = swap_chain->Present(1, 0);
    DX_THROW_IF_FAILED(hr, "Present");

    WaitForPreviousFrame();
}

void HelloWindow::PopulateCommandList()
{
    REQUIRE(!error_status_);
    auto& device = devices_.device;
    auto& command_allocator = devices_.command_allocator;
    auto& render_targets = devices_.render_targets;
    auto& rtv_heap = devices_.rtv_heap;
    auto rtv_descriptor_size = devices_.rtv_descriptor_size;

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    HRESULT hr = command_allocator->Reset();
    DX_THROW_IF_FAILED(hr, "ID3D12CommandAllocator::Reset");

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    hr = command_list_->Reset(command_allocator.Get(), pipeline_state_.Get());
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Reset");

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets[frame_index_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    command_list_->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
        rtv_heap->GetCPUDescriptorHandleForHeapStart(), frame_index_, rtv_descriptor_size);

    // Record commands.
    const float clear_color[] = {0.0f, 0.2f, 0.4f, 1.0f};
    command_list_->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets[frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    command_list_->ResourceBarrier(1, &barrier);

    hr = command_list_->Close();
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");
}

void HelloWindow::WaitForPreviousFrame()
{
    auto& swap_chain = devices_.swap_chain;
    auto& command_queue = devices_.command_queue;

    if (!command_queue || !fence_)
    {
        return;
    }

    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    dx12::utils::WaitForCommandExcuted(command_queue.Get(), fence_.Get(), fence_event_.get(), fence_value_++);
    frame_index_ = swap_chain->GetCurrentBackBufferIndex();
}

void HelloWindow::OnDXError(base::Error const& error) {
    error_status_ = true;

    if (on_error_)
    {
        on_error_(error);
    }
}

} // namespace samples
