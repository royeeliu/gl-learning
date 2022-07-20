#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"
#include "dx_error.hpp"

namespace dx12 {
namespace utils {

template <class V, size_t N>
D3D12_VERTEX_BUFFER_VIEW MakeVertexBufferView(ID3D12Resource* vertex_buffer, V const (&vertices)[N]) noexcept
{
    REQUIRE(vertex_buffer);
    return {
        .BufferLocation = vertex_buffer->GetGPUVirtualAddress(),
        .SizeInBytes = sizeof(vertices),
        .StrideInBytes = sizeof(V),
    };
}

/**
 * @exception DXException
 */
inline void WaitForCommandExcuted(
    ID3D12CommandQueue* queue, ID3D12Fence* fence, HANDLE fence_event, uint64_t fence_value)
{
    // Signal and increment the fence value.
    HRESULT hr = queue->Signal(fence, fence_value);
    DX_THROW_IF_FAILED(hr, "Signal");

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < fence_value)
    {
        hr = fence->SetEventOnCompletion(fence_value, fence_event);
        DX_THROW_IF_FAILED(hr, "SetEventOnCompletion");
        ::WaitForSingleObject(fence_event, INFINITE);
    }
}

inline uint32_t CalcConstantBufferByteSize(uint32_t byte_size)
{
    // Constant buffers must be a multiple of the minimum hardware
    // allocation size (usually 256 bytes).  So round up to nearest
    // multiple of 256.  We do this by adding 255 and then masking off
    // the lower 2 bytes which store all bits < 256.
    // Example: Suppose byteSize = 300.
    // (300 + 255) & ~255
    // 555 & ~255
    // 0x022B & ~0x00ff
    // 0x022B & 0xff00
    // 0x0200
    // 512
    return (byte_size + 255) & ~255;
}

} // namespace utils
} // namespace dx12
