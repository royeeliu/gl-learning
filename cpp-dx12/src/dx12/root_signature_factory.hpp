#pragma once

#include "base/macros.hpp"
#include "d3dx12.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

namespace dx12 {

class D3D12RootSignatureFactory final
{
public:
    D3D12RootSignatureFactory(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
        desc_.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    }

    ComPtr<ID3D12RootSignature> Create() const
    {
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = ::D3D12SerializeRootSignature(&desc_, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        DX_THROW_IF_FAILED(hr, "D3D12SerializeRootSignature");

        ComPtr<ID3D12RootSignature> root_signature;
        hr = device_->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature));
        DX_THROW_IF_FAILED(hr, "CreateRootSignature");
        return root_signature;
    }

private:
    ComPtr<ID3D12Device> device_;
    CD3DX12_ROOT_SIGNATURE_DESC desc_{};
};

} // namespace dx12
