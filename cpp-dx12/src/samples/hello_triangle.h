#pragma once

#include "base/errors.hpp"
#include "base/unique_deleters.hpp"
#include "dx12/dx12_framework.hpp"
#include "sample_base.h"
#include "sample_common.h"

#include <array>
#include <functional>
#include <optional>

namespace samples {

using Microsoft::WRL::ComPtr;

class HelloTriangle final : public SampleBase
{
public:
    explicit HelloTriangle(HWND hwnd, ErrorCallback on_error = nullptr) noexcept;
    ~HelloTriangle() noexcept;

    void Update() noexcept override;
    bool Render() noexcept override;

private:
    void LoadPipeline();
    void LoadAssets();
    void DoRedner();
    void PopulateCommandList();
    void WaitForPreviousFrame();
    void OnDXError(base::Error const& error);

private:
    static constexpr UINT FrameCount = 2;
    static constexpr D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    const HWND hwnd_;
    const ErrorCallback on_error_;
    bool use_warp_device_ = false;
    bool error_status_ = false;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    float aspect_ratio_ = 1.0f;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> swap_chain_;
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12CommandAllocator> command_allocator_;
    ComPtr<ID3D12CommandQueue> command_queue_;
    ComPtr<ID3D12DescriptorHeap> rtv_heap_;
    ComPtr<ID3D12RootSignature> root_signature_;
    ComPtr<ID3D12PipelineState> pipeline_state_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;
    std::array<ComPtr<ID3D12Resource>, FrameCount> render_targets_;
    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissor_rect_{};
    UINT rtv_descriptor_size_ = 0;

    // App resources.
    ComPtr<ID3D12Resource> vertex_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_{};

    // Synchronization objects.
    UINT frame_index_ = 0;
    ComPtr<ID3D12Fence> fence_;
    UINT64 fence_value_ = 0;
    std::unique_ptr<void, base::handle_delete> fence_event_;
};

} // namespace samples
