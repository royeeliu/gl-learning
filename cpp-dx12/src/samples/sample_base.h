#pragma once

#include "base/errors.hpp"
#include <functional>

namespace samples {

class SampleBase
{
public:
    virtual void Update() noexcept = 0;
    virtual bool Render() noexcept = 0;

    virtual ~SampleBase() noexcept {}
};

using ErrorCallback = std::function<void(base::Error const&)>;

} // namespace samples
