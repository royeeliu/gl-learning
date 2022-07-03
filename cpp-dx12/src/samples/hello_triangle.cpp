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
    auto dxgi_factory = dx12::DxgiFactoryCreator().EnableDebugLayerIfOnDebug().Create();
    auto device = dx12::D3D12DeviceFactory(dxgi_factory.Get()).UseWarpAdapter(use_warp_device_).Create();
    auto [command_queue, command_allocator] = dx12::D3D12CommandQueueFactory(device.Get()).Create();

    // Swap chain needs the queue so that it can force a flush on it.
    auto swap_chain1 = dx12::DxgiSwapChainFactory(dxgi_factory.Get(), command_queue.Get())
                           .BufferCount(FrameCount)
                           .Size(width_, height_)
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

void HelloTriangle::LoadAssets()
{
    // Create an empty root signature.
    auto root_signature = dx12::D3D12RootSignatureFactory(device_.Get()).Create();

    // Create the pipeline state, which includes compiling and loading shaders.
    D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
    auto pipeline_state = dx12::D3D12PipelineStateFactory(device_.Get())
                              .RootSignature(root_signature.Get())
                              .InputLayout(input_element_descs)
                              .VertexShaderBytecodeNoCopy(g_basic_vs_bytecode)
                              .PixelShaderBytecodeNoCopy(g_basic_ps_bytecode)
                              .Create();

    // Create the command list.
    ComPtr<ID3D12GraphicsCommandList> command_list;
    HRESULT hr =
        device_->CreateCommandList(0, CommandListType, command_allocator_.Get(), pipeline_state.Get(), IID_PPV_ARGS(&command_list));
    DX_THROW_IF_FAILED(hr, "CreateCommandList");

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    hr = command_list->Close();
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");

    // Create synchronization objects.
    ComPtr<ID3D12Fence> fence;
    hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

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
    hr = device_->CreateCommittedResource(
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

void HelloTriangle::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    HRESULT hr = command_allocator_->Reset();
    DX_THROW_IF_FAILED(hr, "ID3D12CommandAllocator::Reset");

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ASSERT(pipeline_state_);
    hr = command_list_->Reset(command_allocator_.Get(), pipeline_state_.Get());
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Reset");

    // Set necessary state.
    command_list_->SetGraphicsRootSignature(root_signature_.Get());
    command_list_->RSSetViewports(1, &viewport_);
    command_list_->RSSetScissorRects(1, &scissor_rect_);

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets_[frame_index_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    command_list_->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
        rtv_heap_->GetCPUDescriptorHandleForHeapStart(), frame_index_, rtv_descriptor_size_);
    command_list_->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

    // Record commands.
    const float clear_color[] = {0.0f, 0.2f, 0.4f, 1.0f};
    command_list_->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
    command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
    command_list_->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        render_targets_[frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    command_list_->ResourceBarrier(1, &barrier);

    hr = command_list_->Close();
    DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");
}

void HelloTriangle::WaitForPreviousFrame()
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

void HelloTriangle::OnDXError(base::Error const& error)
{
    error_status_ = true;

    if (on_error_)
    {
        on_error_(error);
    }
}

} // namespace samples
