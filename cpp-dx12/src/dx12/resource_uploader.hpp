#pragma once

#include "base/macros.hpp"
#include "d3dx12.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

namespace dx12 {

class ResourceUploader final
{
public:
    explicit ResourceUploader(ID3D12Device* device) noexcept
        : device_{device}
    {
        REQUIRE(device_);
    }

    template<class T>
    ComPtr<ID3D12Resource> Upload(T const* data, size_t element_count) {
        REQUIRE(data);
        REQUIRE(element_count > 0);
        const size_t data_size = sizeof(T) * element_count;

        // Note: using upload heaps to transfer static data like vert buffers is not
        // recommended. Every time the GPU needs it, the upload heap will be marshalled
        // over. Please read up on Default Heap usage. An upload heap is used here for
        // code simplicity and because there are very few verts to actually transfer.
        ComPtr<ID3D12Resource> resource;
        auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(data_size);
        HRESULT hr = device_->CreateCommittedResource(
            &heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&resource));
        DX_THROW_IF_FAILED(hr, "CreateCommittedResource");

        // Copy the triangle data to the vertex buffer.
        UINT8* buffer_begin = nullptr;
        CD3DX12_RANGE read_range(0, 0); // We do not intend to read from this resource on the CPU.
        hr = resource->Map(0, &read_range, reinterpret_cast<void**>(&buffer_begin));
        DX_THROW_IF_FAILED(hr, "Map");
        memcpy(buffer_begin, data, data_size);
        resource->Unmap(0, nullptr);

        return resource;
    }

    template <class T, size_t N>
    ComPtr<ID3D12Resource> Upload(T const (&data)[N])
    {
        return Upload(data, N);
    }

private:
    ComPtr<ID3D12Device> device_;
};

} // namespace dx12
