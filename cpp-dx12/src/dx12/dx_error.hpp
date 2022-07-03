#pragma once

#include "base/com_error.hpp"
#include <winerror.h>

namespace dx12 {

class DXError : public base::ComError
{
public:
    DXError(HRESULT hr, std::u8string message) noexcept
        : ComError{hr, message}
    {
    }

    ~DXError() noexcept override {}

    DXError(DXError&&) noexcept = default;
    DXError& operator=(DXError&&) noexcept = default;

    bool IsDeviceLost() const noexcept
    {
        return HResult() == DXGI_ERROR_DEVICE_REMOVED;
    }
};

inline DXError MakeDXError(HRESULT hr, char const* api)
{
    return DXError{hr, stdx::locale_to_u8(std::format("call {} return 0x{:8X}", api, static_cast<uint32_t>(hr)))};
}

class DXException : public base::ComExceptionBaseImpl<DXException, DXError>
{
    using BaseClass = base::ComExceptionBaseImpl<DXException, DXError>;
public:
    DXException(DXError error)
        : BaseClass{std::move(error)}
    {
    }

    bool IsDeviceLost() const noexcept
    {
        return BaseClass::Error().IsDeviceLost();
    }

    static char const* Name() noexcept
    {
        return "DirectX Exception";
    }
};

__declspec(noreturn) inline void ThrowException(DXError&& error)
{
    throw DXException{std::move(error)};
}

} // namespace dx12


#define DX_MAKE_ERROR(hr, api) (dx12::MakeDXError(hr, api) << MAKE_SOURCE_LOCATION())

#define DX_THROW_IF_FAILED(hr, api)                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (FAILED(hr))                                                                                                \
        {                                                                                                              \
            dx12::ThrowException(std::move(DX_MAKE_ERROR(hr, api)));                                                   \
        }                                                                                                              \
    } while (0)
