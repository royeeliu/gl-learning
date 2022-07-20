#pragma once

#include "base/macros.hpp"
#include "d3dx12.hpp"
#include "dx12_framework.hpp"
#include "dx12_utils.hpp"
#include "dx_error.hpp"
#include "stdx/noncopyable.hpp"

namespace dx12 {

struct IsConstantBuffer
{
    static constexpr bool value = true;
};

struct NotConstantBuffer
{
    static constexpr bool value = false;
};

template<class T, bool IS_CONSTANTE_BUFFER>
class UploadBuffer final : stdx::noncopyable
{
public:
    UploadBuffer(ID3D12Device* device, uint32_t element_count) 
        : element_count_{element_count}
        , element_byte_size_{CalcElementSize()}
        , device_{device}
    {
        REQUIRE(element_count_ > 0);
        REQUIRE(device_);

        auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(element_byte_size_ * element_count_);
        HRESULT hr = device_->CreateCommittedResource(
            &heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&upload_buffer_));
        DX_THROW_IF_FAILED(hr, "CreateCommittedResource");

        CD3DX12_RANGE read_range(0, 0); // We do not intend to read from this resource on the CPU.
        hr = upload_buffer_->Map(0, &read_range, reinterpret_cast<void**>(&mapped_data_));
        DX_THROW_IF_FAILED(hr, "Map");

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    ~UploadBuffer()
    {
        if (upload_buffer_ != nullptr)
        {
            upload_buffer_->Unmap(0, nullptr);
        }
        mapped_data_ = nullptr;
    }

    void CopyData(int element_index, const T& data) noexcept
    {
        REQUIRE((element_index >= 0) && (static_cast<uint32_t>(element_index) < element_count_));
        memcpy_s(&mapped_data_[element_index * element_byte_size_], element_byte_size_, &data, sizeof(T));
    }

    template <int N>
    void CopyData(int element_index, T const (&data)[N])
    {
        REQUIRE((element_index >= 0) && (static_cast<uint32_t>(element_index + N) < element_count_));
        for (int i = 0; i < N; i++)
        {
            CopyData(element_index + i, data[i]);
        }
    }

    ID3D12Resource* Resource() const noexcept
    {
        return upload_buffer_.Get();
    }

    uint32_t ElementByteSize() const noexcept
    {
        return element_byte_size_;
    }

private:
    static constexpr uint32_t CalcElementSize() noexcept
    {
        return IS_CONSTANTE_BUFFER ? utils::CalcConstantBufferByteSize(sizeof(T)) : sizeof(T);
    }

private:
    uint32_t const element_count_;
    uint32_t const element_byte_size_;
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12Resource> upload_buffer_;
    uint8_t* mapped_data_ = nullptr;
};

} // namespace dx12
