#pragma once

#include "base/errors.hpp"
#include "dx12/dx12_framework.hpp"
#include "sample_base.h"
#include "sample_devices.h"
#include "stdx/noncopyable.hpp"

#include <array>
#include <functional>
#include <optional>

namespace samples {

using Microsoft::WRL::ComPtr;

class HelloConstantBuffer final
    : public SampleBase
    , stdx::noncopyable
{
public:
    HelloConstantBuffer(HWND hwnd, ErrorCallback on_error) noexcept;
    ~HelloConstantBuffer() noexcept; 

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
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    struct SceneConstantBuffer
    {
        DirectX::XMFLOAT4 offset;
        float padding[60]; // Padding so the constant buffer is 256-byte aligned.
    };
    static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

    const HWND hwnd_;
    const ErrorCallback on_error_;
    bool use_warp_device_ = false;
    bool error_status_ = false;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    float aspect_ratio_ = 1.0f;

    // Pipeline objects.
    SampleDevices devices_{};
    ComPtr<ID3D12DescriptorHeap> cbv_heap_;
    ComPtr<ID3D12RootSignature> root_signature_;
    ComPtr<ID3D12PipelineState> pipeline_state_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;
    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissor_rect_{};

    // App resources.
    ComPtr<ID3D12Resource> vertex_buffer_;
    ComPtr<ID3D12Resource> constant_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_{};
    SceneConstantBuffer constant_buffer_data_;
    uint8_t* cbv_data_begin_ = nullptr;

    // Synchronization objects.
    ComPtr<ID3D12Fence> fence_;
    std::shared_ptr<void> fence_event_;
    uint64_t fence_value_ = 0;
    uint32_t frame_index_ = 0;
};

} // namespace samples
