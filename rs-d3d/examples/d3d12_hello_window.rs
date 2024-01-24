use rs_dx12::*;
use winit::{
    event::{Event, WindowEvent},
    event_loop::EventLoop,
    window::{Window, WindowBuilder},
};

mod sample {
    use raw_window_handle::RawWindowHandle;
    use windows::{
        core::*,
        Win32::Foundation::*,
        Win32::Graphics::{
            Direct3D::*,
            Direct3D12::*,
            Dxgi::{Common::*, *},
        },
        Win32::System::Threading::*,
    };
    use winit::raw_window_handle::HasWindowHandle;

    use super::*;

    const FRAME_COUNT: u32 = 2;

    #[derive(Debug)]
    pub enum SampleError {
        WindowsError(windows::core::Error),
    }
    impl From<windows::core::Error> for SampleError {
        fn from(e: windows::core::Error) -> Self {
            Self::WindowsError(e)
        }
    }
    type Result<T> = std::result::Result<T, SampleError>;

    pub struct Sample {
        dxgi_factory: IDXGIFactory4,
        device: ID3D12Device,
        resources: Option<Resources>,
    }
    struct Resources {
        command_queue: ID3D12CommandQueue,
        swap_chain: IDXGISwapChain3,
        command_allocator: ID3D12CommandAllocator,
        command_list: ID3D12GraphicsCommandList,
        frame_index: u32,
        render_targets: [ID3D12Resource; FRAME_COUNT as usize],
        rtv_heap: ID3D12DescriptorHeap,
        rtv_descriptor_size: usize,
        viewport: D3D12_VIEWPORT,
        scissor_rect: RECT,
        fence: ID3D12Fence,
        fence_value: u64,
        fence_event: HANDLE,
    }

    impl Sample {
        pub fn new() -> Self {
            let (dxgi_factory, device) = d3d12::DeviceFactory::new().create().unwrap();
            Self {
                dxgi_factory,
                device,
                resources: None,
            }
        }

        pub fn bind_to_window(&mut self, window: &Window) -> Result<()> {
            let hwnd =
                if let RawWindowHandle::Win32(handle) = window.window_handle().unwrap().as_raw() {
                    handle.hwnd
                } else {
                    panic!("Unsupported window handle type");
                };
            let hwnd = HWND(hwnd.into());
            let size = window.inner_size();

            let command_queue: ID3D12CommandQueue = unsafe {
                self.device.CreateCommandQueue(&D3D12_COMMAND_QUEUE_DESC {
                    Type: D3D12_COMMAND_LIST_TYPE_DIRECT,
                    ..Default::default()
                })?
            };

            let swap_chain_desc = DXGI_SWAP_CHAIN_DESC1 {
                BufferCount: FRAME_COUNT,
                Width: size.width,
                Height: size.height,
                Format: DXGI_FORMAT_R8G8B8A8_UNORM,
                BufferUsage: DXGI_USAGE_RENDER_TARGET_OUTPUT,
                SwapEffect: DXGI_SWAP_EFFECT_FLIP_DISCARD,
                SampleDesc: DXGI_SAMPLE_DESC {
                    Count: 1,
                    ..Default::default()
                },
                ..Default::default()
            };

            let swap_chain: IDXGISwapChain3 = unsafe {
                self.dxgi_factory.CreateSwapChainForHwnd(
                    &command_queue,
                    hwnd,
                    &swap_chain_desc,
                    None,
                    None,
                )?
            }
            .cast()?;

            // This sample does not support fullscreen transitions
            unsafe {
                self.dxgi_factory
                    .MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)?;
            }

            let frame_index = unsafe { swap_chain.GetCurrentBackBufferIndex() };

            let rtv_heap: ID3D12DescriptorHeap = unsafe {
                self.device
                    .CreateDescriptorHeap(&D3D12_DESCRIPTOR_HEAP_DESC {
                        NumDescriptors: FRAME_COUNT,
                        Type: D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                        ..Default::default()
                    })
            }?;

            let rtv_descriptor_size = unsafe {
                self.device
                    .GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
            } as usize;
            let rtv_handle = unsafe { rtv_heap.GetCPUDescriptorHandleForHeapStart() };

            let render_targets: [ID3D12Resource; FRAME_COUNT as usize] =
                array_init::try_array_init(|i: usize| -> Result<ID3D12Resource> {
                    let render_target: ID3D12Resource = unsafe { swap_chain.GetBuffer(i as u32) }?;
                    unsafe {
                        self.device.CreateRenderTargetView(
                            &render_target,
                            None,
                            D3D12_CPU_DESCRIPTOR_HANDLE {
                                ptr: rtv_handle.ptr + i * rtv_descriptor_size,
                            },
                        )
                    };
                    Ok(render_target)
                })?;

            let viewport = D3D12_VIEWPORT {
                TopLeftX: 0.0,
                TopLeftY: 0.0,
                Width: size.width as f32,
                Height: size.height as f32,
                MinDepth: D3D12_MIN_DEPTH,
                MaxDepth: D3D12_MAX_DEPTH,
            };

            let scissor_rect = RECT {
                left: 0,
                top: 0,
                right: size.width as i32,
                bottom: size.height as i32,
            };

            let command_allocator = unsafe {
                self.device
                    .CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT)
            }?;
            let command_list: ID3D12GraphicsCommandList = unsafe {
                self.device.CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    &command_allocator,
                    None,
                )
            }?;
            unsafe {
                command_list.Close()?;
            };

            let fence: ID3D12Fence = unsafe { self.device.CreateFence(0, D3D12_FENCE_FLAG_NONE) }?;
            let fence_value = 1;
            let fence_event = unsafe { CreateEventA(None, false, false, None)? };

            self.resources = Some(Resources {
                command_queue,
                swap_chain,
                frame_index,
                render_targets,
                rtv_heap,
                rtv_descriptor_size,
                viewport,
                scissor_rect,
                command_allocator,
                command_list,
                fence,
                fence_value,
                fence_event,
            });

            Ok(())
        }

        pub fn render(&mut self) {
            if let Some(resources) = &mut self.resources {
                Self::populate_command_list(resources).unwrap();

                let command_list = Some(resources.command_list.cast().unwrap());
                unsafe {
                    resources.command_queue.ExecuteCommandLists(&[command_list]);
                    resources.swap_chain.Present(1, 0).ok().unwrap();
                }

                Self::wait_for_previous_frame(resources);
            }
        }

        fn populate_command_list(resources: &Resources) -> Result<()> {
            let command_list = &resources.command_list;
            // Indicate that the back buffer will be used as a render target.
            let barrier = Self::transition_barrier(
                &resources.render_targets[resources.frame_index as usize],
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
            );
            let rtv_handle = D3D12_CPU_DESCRIPTOR_HANDLE {
                ptr: unsafe { resources.rtv_heap.GetCPUDescriptorHandleForHeapStart() }.ptr
                    + resources.frame_index as usize * resources.rtv_descriptor_size,
            };

            unsafe {
                resources.command_allocator.Reset()?;
                command_list.Reset(&resources.command_allocator, None)?;

                // Set necessary state.
                command_list.RSSetViewports(&[resources.viewport]);
                command_list.RSSetScissorRects(&[resources.scissor_rect]);
                command_list.ResourceBarrier(&[barrier]);
                command_list.OMSetRenderTargets(1, Some(&rtv_handle), false, None);

                // Record commands.
                command_list.ClearRenderTargetView(
                    rtv_handle,
                    &[0.0_f32, 0.2_f32, 0.4_f32, 1.0_f32],
                    None,
                );
                command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                command_list.DrawInstanced(3, 1, 0, 0);

                // Indicate that the back buffer will now be used to present.
                command_list.ResourceBarrier(&[Self::transition_barrier(
                    &resources.render_targets[resources.frame_index as usize],
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    D3D12_RESOURCE_STATE_PRESENT,
                )]);
                command_list.Close()?
            }
            Ok(())
        }

        fn transition_barrier(
            resource: &ID3D12Resource,
            state_before: D3D12_RESOURCE_STATES,
            state_after: D3D12_RESOURCE_STATES,
        ) -> D3D12_RESOURCE_BARRIER {
            D3D12_RESOURCE_BARRIER {
                Type: D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                Flags: D3D12_RESOURCE_BARRIER_FLAG_NONE,
                Anonymous: D3D12_RESOURCE_BARRIER_0 {
                    Transition: std::mem::ManuallyDrop::new(D3D12_RESOURCE_TRANSITION_BARRIER {
                        pResource: unsafe { std::mem::transmute_copy(resource) },
                        StateBefore: state_before,
                        StateAfter: state_after,
                        Subresource: D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                    }),
                },
            }
        }

        fn wait_for_previous_frame(resources: &mut Resources) {
            // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST
            // PRACTICE. This is code implemented as such for simplicity. The
            // D3D12HelloFrameBuffering sample illustrates how to use fences for
            // efficient resource usage and to maximize GPU utilization.

            // Signal and increment the fence value.
            let fence = resources.fence_value;

            unsafe { resources.command_queue.Signal(&resources.fence, fence) }
                .ok()
                .unwrap();

            resources.fence_value += 1;

            // Wait until the previous frame is finished.
            if unsafe { resources.fence.GetCompletedValue() } < fence {
                unsafe {
                    resources
                        .fence
                        .SetEventOnCompletion(fence, resources.fence_event)
                }
                .ok()
                .unwrap();

                unsafe { WaitForSingleObject(resources.fence_event, INFINITE) };
            }

            resources.frame_index = unsafe { resources.swap_chain.GetCurrentBackBufferIndex() };
        }
    }
}

fn start_event_loop(
    sample: &mut sample::Sample,
    window: &Window,
    event_loop: EventLoop<()>,
) -> std::result::Result<(), impl std::error::Error> {
    event_loop.run(move |event, elwt| {
        //println!("{event:?}");
        match event {
            Event::WindowEvent { event, window_id } if window_id == window.id() => match event {
                WindowEvent::CloseRequested => elwt.exit(),
                WindowEvent::RedrawRequested => {
                    window.pre_present_notify();
                    sample.render();
                }
                _ => (),
            },
            Event::AboutToWait => {
                window.request_redraw();
            }

            _ => (),
        }
    })
}

fn main() -> std::result::Result<(), impl std::error::Error> {
    let event_loop = EventLoop::new().unwrap();
    let window = WindowBuilder::new()
        .with_title("Hello, Window!")
        .with_inner_size(winit::dpi::LogicalSize::new(800, 600))
        .build(&event_loop)
        .unwrap();
    let mut sample = sample::Sample::new();
    sample.bind_to_window(&window).unwrap();
    start_event_loop(&mut sample, &window, event_loop)
}
