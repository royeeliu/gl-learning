#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

#include <tuple>

namespace dx12 {

class D3D12CommandListFactory final
{
public:
    D3D12CommandListFactory(
        ID3D12Device* device, ID3D12CommandAllocator* allocator, ID3D12PipelineState* pipeline_state = nullptr) noexcept
        : device_{device}
        , allocator_{allocator}
        , pipeline_state_{pipeline_state}
    {
        REQUIRE(device_);
        REQUIRE(allocator_);
    }

    D3D12CommandListFactory& Type(D3D12_COMMAND_LIST_TYPE type) noexcept
    {
        type_ = type;
    }

    ComPtr<ID3D12GraphicsCommandList> Create() const
    {
        ComPtr<ID3D12GraphicsCommandList> command_list; 
        HRESULT hr = device_->CreateCommandList(
            0, type_, allocator_.Get(), pipeline_state_.Get(), IID_PPV_ARGS(&command_list));
        DX_THROW_IF_FAILED(hr, "CreateCommandList");

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        hr = command_list->Close();
        DX_THROW_IF_FAILED(hr, "ID3D12GraphicsCommandList::Close");
        return command_list;
    }

private:
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12CommandAllocator> allocator_;
    ComPtr<ID3D12PipelineState> pipeline_state_;
    D3D12_COMMAND_LIST_TYPE type_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
};

} // namespace dx12
