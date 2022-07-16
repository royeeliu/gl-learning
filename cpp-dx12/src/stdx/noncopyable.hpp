#pragma once

namespace stdx {

class noncopyable
{
protected:
    noncopyable() noexcept = default;
    ~noncopyable() noexcept = default;

private:
    noncopyable(noncopyable const&) = delete;
    noncopyable& operator=(noncopyable const&) = delete;
};

} // namespace stdx
