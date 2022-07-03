#pragma once

namespace samples {

class SampleBase
{
public:
    virtual void Update() noexcept = 0;
    virtual bool Render() noexcept = 0;

    virtual ~SampleBase() noexcept {}
};

} // namespace samples
