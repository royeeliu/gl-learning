#pragma once

#include "sample_common.h"
#include "dx12/dx12_framework.hpp"

#include <array>

namespace samples {

struct SampleDevices
{
    // Pipeline objects.
    ComPtr<ID3D12Device> device;
    ComPtr<IDXGISwapChain3> swap_chain;
    ComPtr<ID3D12CommandAllocator> command_allocator;
    ComPtr<ID3D12CommandQueue> command_queue;
    ComPtr<ID3D12DescriptorHeap> rtv_heap;
    UINT rtv_descriptor_size = 0;
    std::array<ComPtr<ID3D12Resource>, SWAP_CHAIN_BUFFER_COUNT> render_targets;

public:
    void Initialize(HWND hwnd, uint32_t width, uint32_t height);
};

} // namespace samples
