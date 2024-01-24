use windows::{
    core::*, Win32::Graphics::Direct3D::*, Win32::Graphics::Direct3D12::*, Win32::Graphics::Dxgi::*,
};

pub struct DeviceFactory {
    factory: Option<IDXGIFactory4>,
    enable_debug_layer: bool,
    use_warp: bool,
}

impl DeviceFactory {
    pub fn new() -> Self {
        let enable_debug = if cfg!(debug_assertions) { true } else { false };
        Self {
            factory: None,
            enable_debug_layer: enable_debug,
            use_warp: false,
        }
    }

    pub fn dxgi_factory(&mut self, factory: IDXGIFactory4) -> &mut Self {
        self.factory = Some(factory);
        self
    }

    pub fn enable_debug_layer(&mut self, enable: bool) -> &mut Self {
        self.enable_debug_layer = enable;
        self
    }

    pub fn use_warp_adapter(&mut self, use_warp: bool) -> &mut Self {
        self.use_warp = use_warp;
        self
    }

    pub fn create(mut self) -> Result<(IDXGIFactory4, ID3D12Device)> {
        if self.enable_debug_layer {
            unsafe {
                // Enable the debug layer (requires the Graphics Tools "optional feature").
                // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                let mut debug: Option<ID3D12Debug> = None;
                if let Some(debug) = D3D12GetDebugInterface(&mut debug).ok().and(debug) {
                    debug.EnableDebugLayer();
                }
            }
        }

        let factory = match self.factory.take() {
            Some(factory) => factory,
            None => {
                let flags = if self.enable_debug_layer {
                    DXGI_CREATE_FACTORY_DEBUG
                } else {
                    0
                };
                unsafe { CreateDXGIFactory2(flags) }?
            }
        };

        let adapter = if self.use_warp {
            unsafe { factory.EnumWarpAdapter()? }
        } else {
            self.get_hardware_adapter(&factory)?
        };

        let mut device: Option<ID3D12Device> = None;
        unsafe { D3D12CreateDevice(&adapter, D3D_FEATURE_LEVEL_11_0, &mut device) }?;
        Ok((factory, device.unwrap()))
    }

    fn get_hardware_adapter(&self, factory: &IDXGIFactory4) -> Result<IDXGIAdapter1> {
        for i in 0.. {
            let adapter = unsafe { factory.EnumAdapters1(i)? };

            let mut desc = Default::default();
            unsafe { adapter.GetDesc1(&mut desc)? };

            if (DXGI_ADAPTER_FLAG(desc.Flags as i32) & DXGI_ADAPTER_FLAG_SOFTWARE)
                != DXGI_ADAPTER_FLAG_NONE
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't
            // create the actual device yet.
            if unsafe {
                D3D12CreateDevice(
                    &adapter,
                    D3D_FEATURE_LEVEL_11_0,
                    std::ptr::null_mut::<Option<ID3D12Device>>(),
                )
            }
            .is_ok()
            {
                return Ok(adapter);
            }
        }
        unreachable!()
    }
}
