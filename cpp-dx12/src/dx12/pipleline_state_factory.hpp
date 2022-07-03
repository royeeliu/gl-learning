#pragma once

#include "base/macros.hpp"
#include "d3dx12.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

#include <vector>

namespace dx12 {

class D3D12PipelineStateFactory final
{
public:
    D3D12PipelineStateFactory(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
        desc_.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        desc_.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        desc_.DepthStencilState.DepthEnable = FALSE;
        desc_.DepthStencilState.StencilEnable = FALSE;
        desc_.SampleMask = UINT_MAX;
        desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc_.NumRenderTargets = 1;
        desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc_.SampleDesc.Count = 1;
    }

    D3D12PipelineStateFactory& RootSignature(ID3D12RootSignature* root_signature) noexcept
    {
        root_signature_ = root_signature;
        return *this;
    }

    D3D12PipelineStateFactory& InputLayout(D3D12_INPUT_ELEMENT_DESC  const* element_descs, uint32_t count) {
        REQUIRE(element_descs);
        REQUIRE(count > 0);
        input_element_descs_.clear();
        for (uint32_t i = 0; i < count; i++)
        {
            input_element_descs_.push_back(element_descs[i]);
        }
        return *this;
    }

    template <uint32_t Size>
    D3D12PipelineStateFactory& InputLayout(D3D12_INPUT_ELEMENT_DESC const (&element_descs)[Size])
    {
        return InputLayout(element_descs, Size);
    }

    D3D12PipelineStateFactory& VertexShaderBytecode(void const* data, uint32_t size) {
        REQUIRE(data);
        REQUIRE(size > 0);
        vertex_shader_ = CreateBlob(data, size);
        return *this;
    }

    template <uint32_t Size>
    D3D12PipelineStateFactory& VertexShaderBytecode(uint8_t const (&data)[Size])
    {
        return VertexShaderBytecode(data, Size);
    }

    D3D12PipelineStateFactory& VertexShaderBytecodeNoCopy(void const* data, uint32_t size) noexcept
    {
        REQUIRE(data);
        REQUIRE(size > 0);
        desc_.VS = {data, size};
        return *this;
    }

    template <uint32_t Size>
    D3D12PipelineStateFactory& VertexShaderBytecodeNoCopy(uint8_t const (&data)[Size]) noexcept
    {
        return VertexShaderBytecodeNoCopy(data, Size);
    }

    D3D12PipelineStateFactory& PixelShaderBytecode(void const* data, uint32_t size)
    {
        REQUIRE(data);
        REQUIRE(size > 0);
        vertex_shader_ = CreateBlob(data, size);
        return *this;
    }

    template <uint32_t Size>
    D3D12PipelineStateFactory& PixelShaderBytecode(uint8_t const (&data)[Size])
    {
        return PixelShaderBytecode(data, Size);
    }

    D3D12PipelineStateFactory& PixelShaderBytecodeNoCopy(void const* data, uint32_t size) noexcept
    {
        REQUIRE(data);
        REQUIRE(size > 0);
        desc_.PS = {data, size};
        return *this;
    }

    template <uint32_t Size>
    D3D12PipelineStateFactory& PixelShaderBytecodeNoCopy(uint8_t const (&data)[Size]) noexcept
    {
        return PixelShaderBytecodeNoCopy(data, Size);
    }

    ComPtr<ID3D12PipelineState> Create() const
    {
        auto desc = desc_;
        desc.pRootSignature = root_signature_.Get();
        desc.InputLayout.pInputElementDescs = input_element_descs_.data();
        desc.InputLayout.NumElements = static_cast<UINT>(input_element_descs_.size());

        if (vertex_shader_)
        {
            desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader_.Get());
        }
        if (pixel_shader_)
        {
            desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader_.Get());
        }

        ComPtr<ID3D12PipelineState> pipeline_state;
        HRESULT hr = device_->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline_state));
        DX_THROW_IF_FAILED(hr, "CreateGraphicsPipelineState");
        return pipeline_state;
    }

private:
    ComPtr<ID3DBlob> CreateBlob(void const* data, uint32_t size) {
        ComPtr<ID3DBlob> blob;
        HRESULT hr = ::D3DCreateBlob(size, blob.GetAddressOf());
        DX_THROW_IF_FAILED(hr, "D3DCreateBlob");
        memcpy_s(blob->GetBufferPointer(), blob->GetBufferSize(), data, size);
        return blob;
    }

private:
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12RootSignature> root_signature_;
    ComPtr<ID3DBlob> vertex_shader_;
    ComPtr<ID3DBlob> pixel_shader_;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_element_descs_;
};

} // namespace dx12
