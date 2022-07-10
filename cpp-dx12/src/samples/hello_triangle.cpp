#include "hello_triangle.h"
#include "dx12/d3dx12.hpp"
#include "dx12/dx12_factories.hpp"
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

    // Create an empty root signature.
    auto root_signature = dx12::D3D12RootSignatureFactory(device.Get()).Create();

    // Create the pipeline state, which includes compiling and loading shaders.
    D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    auto pipeline_state = dx12::D3D12PipelineStateFactory(device.Get())
                              .RootSignature(root_signature.Get())
                              .InputLayout(input_element_descs)
                              .VertexShaderBytecodeNoCopy(g_basic_vs_bytecode)
                              .PixelShaderBytecodeNoCopy(g_basic_ps_bytecode)
                              .Create();

    // Create the command list.
    auto command_list =
        dx12::D3D12CommandListFactory(device.Get(), command_allocator.Get(), pipeline_state.Get()).Create();

    // Create synchronization objects.
    ComPtr<ID3D12Fence> fence;
    HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    // Create an event handle to use for frame synchronization.
    std::unique_ptr<void, base::handle_delete> fence_event{::CreateEvent(nullptr, FALSE, FALSE, nullptr)};
    if (fence_event == nullptr)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        DX_THROW_IF_FAILED(hr, "CreateEvent");
    }

    // Create the vertex buffer.
    // Define the geometry for a triangle.
    Vertex triangle_vertices[] = {
        {{0.0f, 0.25f * aspect_ratio_, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.25f, -0.25f * aspect_ratio_, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, -0.25f * aspect_ratio_, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}};

    const UINT vertex_buffer_size = sizeof(triangle_vertices);

    // Note: using upload heaps to transfer static data like vert buffers is not
    // recommended. Every time the GPU needs it, the upload heap will be marshalled
    // over. Please read up on Default Heap usage. An upload heap is used here for
    // code simplicity and because there are very few verts to actually transfer.
    ComPtr<ID3D12Resource> vertex_buffer;
    auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto vertex_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);
    hr = device->CreateCommittedResource(
        &heap_properties, D3D12_HEAP_FLAG_NONE, &vertex_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&vertex_buffer));
    DX_THROW_IF_FAILED(hr, "CreateCommittedResource");

    // Copy the triangle data to the vertex buffer.
    UINT8* vertex_data_begin = nullptr;
    CD3DX12_RANGE read_range(0, 0); // We do not intend to read from this resource on the CPU.
    hr = vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin));
    DX_THROW_IF_FAILED(hr, "Map");
    memcpy(vertex_data_begin, triangle_vertices, sizeof(triangle_vertices));
    vertex_buffer->Unmap(0, nullptr);

    root_signature_ = std::move(root_signature);
    pipeline_state_ = std::move(pipeline_state);
    command_list_ = std::move(command_list);
    fence_ = std::move(fence);
    vertex_buffer_ = std::move(vertex_buffer);
    fence_value_ = 1;

    // Initialize the vertex buffer view.
    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view_.SizeInBytes = vertex_buffer_size;

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

    // Signal and increment the fence value.
    const UINT64 fence = fence_value_;
    HRESULT hr = command_queue->Signal(fence_.Get(), fence);
    DX_THROW_IF_FAILED(hr, "Signal");
    fence_value_++;

    // Wait until the previous frame is finished.
    if (fence_->GetCompletedValue() < fence)
    {
        hr = fence_->SetEventOnCompletion(fence, fence_event_.get());
        DX_THROW_IF_FAILED(hr, "Signal");
        ::WaitForSingleObject(fence_event_.get(), INFINITE);
    }

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
