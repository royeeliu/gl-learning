#pragma once

#include "base/macros.hpp"
#include "dx12_framework.hpp"

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

} // namespace utils
} // namespace dx12
