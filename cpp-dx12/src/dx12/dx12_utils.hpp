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

} // namespace utils
} // namespace dx12
