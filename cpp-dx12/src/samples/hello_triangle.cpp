#include "hello_triangle.h"
#include "dx12/d3dx12.hpp"
#include "dx12/dx12_factories.hpp"
#include "dx12/dx12_utils.hpp"
#include "dx12/resource_uploader.hpp"
#include "generated/shaders/basic_ps.h"
#include "generated/shaders/basic_vs.h"

namespace samples {

HelloTriangle::HelloTriangle(HWND hwnd, ErrorCallback on_error) noexcept
    : hwnd_{hwnd}
    , on_error_{std::move(on_error)}
{
    REQUIRE(::IsWindow(hwnd_));

    RECT rect{};
    ::GetClientRect(hwnd_, &rect);
    width_ = rect.right - rect.left;
    height_ = rect.bottom - rect.top;
    viewport_ = CD3DX12_VIEWPORT{0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_)};
    scissor_rect_ = D3D12_RECT{0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
    aspect_ratio_ = static_cast<float>(width_) / static_cast<float>(height_);

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

HelloTriangle::~HelloTriangle() noexcept
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

void HelloTriangle::Update() noexcept {}

bool HelloTriangle::Render() noexcept
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

void HelloTriangle::LoadPipeline()
{
    devices_.Initialize(hwnd_, width_, height_);
    frame_index_ = devices_.swap_chain->GetCurrentBackBufferIndex();
}

void HelloTriangle::LoadAssets()
{
    auto& device = devices_.device;
    auto& command_allocator = devices_.command_allocator;
    auto& vs_bytecode = g_basic_vs_bytecode;
    auto& ps_bytecode = g_basic_ps_bytecode;

    D3D12_INPUT_ELEMENT_DESC const input_element_descs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    // Define the geometry for a triangle.
    Vertex triangle_vertices[] = {
        {{0.0f, 0.25f * aspect_ratio_, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * aspect_ratio_, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * aspect_ratio_, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };

    // Create an empty root signature.
    root_signature_ = dx12::D3D12RootSignatureFactory(device.Get()).Create();

    // Create the pipeline state, which includes compiling and loading shaders.
    pipeline_state_ = dx12::D3D12PipelineStateFactory(device.Get())
                          .RootSignature(root_signature_.Get())
                          .InputLayout(input_element_descs)
                          .VertexShaderBytecodeNoCopy(vs_bytecode)
                          .PixelShaderBytecodeNoCopy(ps_bytecode)
                          .Create();

    // Create the command list.
    command_list_ =
        dx12::D3D12CommandListFactory(device.Get(), command_allocator.Get(), pipeline_state_.Get()).Create();

    // Create synchronization objects.
    std::tie(fence_, fence_event_) = dx12::D3D12FenceFactory(device.Get()).Create();

    // Create the vertex buffer.
    vertex_buffer_ = dx12::ResourceUploader(device.Get()).Upload(triangle_vertices);
    vertex_buffer_view_ = dx12::utils::MakeVertexBufferView(vertex_buffer_.Get(), triangle_vertices);

    fence_value_ = 1;

    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    WaitForPreviousFrame();
}

void HelloTriangle::DoRedner()
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

void HelloTriangle::PopulateCommandList()
{
    REQUIRE(!error_status_);
    auto& device = devices_.device;
    auto& command_allocator = devices_.command_allocator;
    auto& rtv_heap = devices_.rtv_heap;
    auto& render_targets = devices_.render_targets;
    auto rtv_descriptor_size = devices_.rtv_descriptor_size;

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    HRESULT hr = command_allocator->Reset();
    DX_THROW_IF_FAILED(hr, "ID3D12CommandAllocator::Reset");

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ASSERT(pipeline_state_);
    hr = command_list_->Reset(command_allocator.Get(), pipeline_state_.Get());
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Reset");

    // Set necessary state.
    command_list_->SetGraphicsRootSignature(root_signature_.Get());
    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets[frame_index_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    command_list_->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
        rtv_heap->GetCPUDescriptorHandleForHeapStart(), frame_index_, rtv_descriptor_size);
    command_list_->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

    // Record commands.
    const float clear_color[] = {0.0f, 0.2f, 0.4f, 1.0f};
    command_list_->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
    command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
    command_list_->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets[frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    command_list_->ResourceBarrier(1, &barrier);

    hr = command_list_->Close();
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");
}

void HelloTriangle::WaitForPreviousFrame()
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

void HelloTriangle::OnDXError(base::Error const& error)
{
    error_status_ = true;

    if (on_error_)
    {
        on_error_(error);
    }
}

} // namespace samples
