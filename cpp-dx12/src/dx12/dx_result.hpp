#pragma once

#include "base/result.hpp"
#include "dx_error.hpp"

namespace dx12 {

template <class T>
struct Result : base::Result<T, DXError>
{
};

} // namespace dx12
