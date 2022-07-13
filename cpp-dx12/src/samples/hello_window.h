#pragma once

#include "base/deleters.hpp"
#include "base/errors.hpp"
#include "dx12/dx12_framework.hpp"
#include "sample_base.h"
#include "sample_common.h"
#include "sample_devices.h"

#include <array>
#include <functional>
#include <optional>

namespace samples {

class HelloWindow final : public SampleBase
{
public:
    explicit HelloWindow(HWND hwnd, ErrorCallback on_error = nullptr) noexcept;
    ~HelloWindow() noexcept;

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
    const HWND hwnd_;
    const ErrorCallback on_error_;
    bool use_warp_device_ = false;
    bool error_status_ = false;
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    // Pipeline objects.
    SampleDevices devices_{};
    ComPtr<ID3D12PipelineState> pipeline_state_;
    ComPtr<ID3D12GraphicsCommandList> command_list_;

    // Synchronization objects.
    ComPtr<ID3D12Fence> fence_;
    std::shared_ptr<void> fence_event_;
    uint64_t fence_value_ = 0;
    uint32_t frame_index_ = 0;
};

} // namespace samples
