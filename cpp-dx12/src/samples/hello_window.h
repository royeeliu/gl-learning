#pragma once

#include "base/unique_deleters.h"
#include "dx12/dx12_framework.h"
#include "dx12/dx_error.hpp"

#include <array>
#include <functional>
#include <optional>

namespace samples {

using Microsoft::WRL::ComPtr;

class HelloWindow final
{
    using ErrorCallback = std::function<void(base::Error const&)>;

public:
    explicit HelloWindow(HWND hwnd, ErrorCallback on_error = nullptr) noexcept;
    ~HelloWindow() noexcept;

    void Update() noexcept;
    bool Render() noexcept;

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

    const HWND hwnd_;
    const ErrorCallback on_error_;
    bool use_warp_device_ = false;
    bool error_status_ = false;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> swap_chain_;
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12CommandAllocator> command_allocator_;
    ComPtr<ID3D12CommandQueue> command_queue_;
    ComPtr<ID3D12DescriptorHeap> rtv_heap_;
    ComPtr<ID3D12PipelineState> pipeline_state_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;
    std::array<ComPtr<ID3D12Resource>, FrameCount> render_targets_;
    UINT rtv_descriptor_size_ = 0;

    // Synchronization objects.
    UINT frame_index_ = 0;
    ComPtr<ID3D12Fence> fence_;
    UINT64 fence_value_ = 0;
    std::unique_ptr<void, base::handle_delete> fence_event_;
};

} // namespace samples
