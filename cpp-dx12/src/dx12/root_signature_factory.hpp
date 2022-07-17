#pragma once

#include "base/macros.hpp"
#include "d3dx12.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

#include <optional>

namespace dx12 {

class D3D12RootSignatureFactory final
{
public:
    D3D12RootSignatureFactory(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
    }

    D3D12RootSignatureFactory& SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) noexcept {
        flags_ = flags;
        return *this;
    }

    D3D12RootSignatureFactory& AddFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) noexcept {
        flags_ |= flags;
        return *this;
    }

    D3D12RootSignatureFactory& SetParametersNoCopy(
        D3D12_ROOT_PARAMETER const* parameters, uint32_t count) noexcept
    {
        REQUIRE(parameters);
        REQUIRE(count > 0);
        parameters_ = parameters;
        paramters_count_ = count;
        return *this;
    }

    template <uint32_t N>
    D3D12RootSignatureFactory& SetParametersNoCopy(D3D12_ROOT_PARAMETER const (&parameters)[N]) noexcept
    {
        return SetParametersNoCopy(parameters, N);
    }

    template <uint32_t N>
    D3D12RootSignatureFactory& SetParametersNoCopy(CD3DX12_ROOT_PARAMETER const (&parameters)[N]) noexcept
    {
        return SetParametersNoCopy(parameters, N);
    }

    ComPtr<ID3D12RootSignature> Create() const
    {
        CD3DX12_ROOT_SIGNATURE_DESC desc{};
        desc.Init(paramters_count_, parameters_, 0, nullptr, flags_);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;

        try
        {
            HRESULT hr = ::D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
            DX_THROW_IF_FAILED(hr, "D3D12SerializeRootSignature");
        }
        catch (DXException& ex)
        {
            ex << base::MakeErrorInfo(u8"serialize error", reinterpret_cast<char*>(error->GetBufferPointer()));
            throw;
        }

        ComPtr<ID3D12RootSignature> root_signature;
        HRESULT hr = device_->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature));
        DX_THROW_IF_FAILED(hr, "CreateRootSignature");
        return root_signature;
    }

private:
    ComPtr<ID3D12Device> device_;
    D3D12_ROOT_SIGNATURE_FLAGS flags_ = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    D3D12_ROOT_PARAMETER const* parameters_ = nullptr;
    uint32_t paramters_count_ = 0;
};


class D3D12VersionedRootSignatureFactory final
{
public:
    D3D12VersionedRootSignatureFactory(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
    }

    D3D12VersionedRootSignatureFactory& TryVersion_1_1() noexcept {
        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        feature_data_.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data_, sizeof(feature_data_));
        if (FAILED(hr))
        {
            feature_data_.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }
        return *this;
    }

    D3D12VersionedRootSignatureFactory& SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) noexcept
    {
        flags_ = flags;
        return *this;
    }

    D3D12VersionedRootSignatureFactory& AddFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) noexcept
    {
        flags_ |= flags;
        return *this;
    }

    D3D12VersionedRootSignatureFactory& SetParametersNoCopy(
        D3D12_ROOT_PARAMETER1 const* parameters, uint32_t count) noexcept
    {
        REQUIRE(parameters);
        REQUIRE(count > 0);
        parameters_ = parameters;
        paramters_count_ = count;
        return *this;
    }

    template <uint32_t N>
    D3D12VersionedRootSignatureFactory& SetParametersNoCopy(D3D12_ROOT_PARAMETER1 const (&parameters)[N]) noexcept
    {
        return SetParametersNoCopy(parameters, N);
    }

    template <uint32_t N>
    D3D12VersionedRootSignatureFactory& SetParametersNoCopy(CD3DX12_ROOT_PARAMETER1 const (&parameters)[N]) noexcept
    {
        return SetParametersNoCopy(parameters, N);
    }

    ComPtr<ID3D12RootSignature> Create() const
    {
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc{};
        desc.Init_1_1(paramters_count_, parameters_, 0, nullptr, flags_);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;

        try
        {
            HRESULT hr =
                ::D3DX12SerializeVersionedRootSignature(&desc, feature_data_.HighestVersion, &signature, &error);
            DX_THROW_IF_FAILED(hr, "D3DX12SerializeVersionedRootSignature");
        }
        catch (DXException& ex)
        {
            ex << base::MakeErrorInfo(u8"serialize error", reinterpret_cast<char*>(error->GetBufferPointer()));
            throw;
        }

        ComPtr<ID3D12RootSignature> root_signature;
        HRESULT hr = device_->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature));
        DX_THROW_IF_FAILED(hr, "CreateRootSignature");

        return root_signature;
    }

private:
    ComPtr<ID3D12Device> device_;
    D3D12_ROOT_SIGNATURE_FLAGS flags_ = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data_{.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0};
    D3D12_ROOT_PARAMETER1 const* parameters_ = nullptr;
    uint32_t paramters_count_ = 0;
};

} // namespace dx12
