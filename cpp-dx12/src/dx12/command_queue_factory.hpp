#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

#include <tuple>

namespace dx12 {

class D3D12CommandQueueFactory final
{
public:
    D3D12CommandQueueFactory(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
        desc_.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc_.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    }

    ComPtr<ID3D12CommandQueue> CreateCommandQueue() const
    {
        ComPtr<ID3D12CommandQueue> command_queue;
        HRESULT hr = device_->CreateCommandQueue(&desc_, IID_PPV_ARGS(&command_queue));
        DX_THROW_IF_FAILED(hr, "CreateCommandQueue");
        return command_queue;
    }

    ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() const
    {
        ComPtr<ID3D12CommandAllocator> command_allocator;
        HRESULT hr = device_->CreateCommandAllocator(desc_.Type, IID_PPV_ARGS(&command_allocator));
        DX_THROW_IF_FAILED(hr, "CreateCommandAllocator");
        return command_allocator;
    }

    std::tuple<ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12CommandAllocator>> Create() const
    {
        return {CreateCommandQueue(), CreateCommandAllocator()};
    }

private:
    ComPtr<ID3D12Device> device_;
    D3D12_COMMAND_QUEUE_DESC desc_ = {};
};

} // namespace dx12
