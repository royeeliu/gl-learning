#pragma once

#pragma once

#include "base/deleters.hpp"
#include "base/macros.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

#include <tuple>

namespace dx12 {

class D3D12FenceFactory final
{
public:
    using FenceEvent = std::shared_ptr<void>;

    D3D12FenceFactory(
        ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
    }

    D3D12FenceFactory& SetFlags(D3D12_FENCE_FLAGS flags) noexcept
    {
        flags_ = flags;
    }

    std::tuple<ComPtr<ID3D12Fence>, FenceEvent> Create() const
    {
        ComPtr<ID3D12Fence> fence;
        HRESULT hr = device_->CreateFence(0, flags_, IID_PPV_ARGS(&fence));
        DX_THROW_IF_FAILED(hr, "CreateFence");

        // Create an event handle to use for frame synchronization.
        HANDLE fence_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fence_event == nullptr)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DX_THROW_IF_FAILED(hr, "CreateEvent");
        }
        return {fence, std::shared_ptr<void>{fence_event, base::handle_delete{}}};
    }

private:
    ComPtr<ID3D12Device> device_;
    D3D12_FENCE_FLAGS flags_ = D3D12_FENCE_FLAG_NONE;
};

} // namespace dx12
