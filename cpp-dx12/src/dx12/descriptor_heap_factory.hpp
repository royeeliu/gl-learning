#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

namespace dx12 {

class D3D12DescriptorHeapFactory final
{
public:
    D3D12DescriptorHeapFactory(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
        desc_.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }

    D3D12DescriptorHeapFactory& Type(D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept
    {
        desc_.Type = type;
        return *this;
    }

    D3D12DescriptorHeapFactory& NumDescriptors(uint32_t num) noexcept
    {
        desc_.NumDescriptors = num;
        return *this;
    }

    ComPtr<ID3D12DescriptorHeap> Create() const
    {
        ComPtr<ID3D12DescriptorHeap> heap;
        HRESULT hr = device_->CreateDescriptorHeap(&desc_, IID_PPV_ARGS(&heap));
        DX_THROW_IF_FAILED(hr, "CreateDescriptorHeap");
        return heap;
    }

private:
    ComPtr<ID3D12Device> device_;
    D3D12_DESCRIPTOR_HEAP_DESC desc_ = {};
};

} // namespace dx12

