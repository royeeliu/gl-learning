#pragma once

#include "errors.hpp"
#include <string>

namespace base {

template <
    class T, class TError = base::Error, class TBase = Exception,
    class = typename std::enable_if<
        std::is_base_of<base::Error, TError>::value && std::is_base_of<Exception, TBase>::value, void>::type>
class ExceptionBaseImpl : public TBase
{
public:
    using ErrorType = TError;

    explicit ExceptionBaseImpl(ErrorType error) noexcept
        : TBase{T::Name()}
        , error_{std::move(error)}
    {
    }

    ErrorType const& Error() const noexcept
    {
        return error_;
    }

    std::error_code const& Code() const noexcept override
    {
        return error_.Code();
    }

    std::u8string const& Message() const noexcept override
    {
        return error_.Message();
    }

    void PushSourceLocation(SourceLocation const& location) noexcept override
    {
        return error_.PushSourceLocation(location);
    }

    void PushInfo(ErrorInfo info) noexcept override
    {
        return error_.PushInfo(std::move(info));
    }

    std::u8string GetDiagnosticInfo(char8_t const* prefix, char8_t const* suffix) const override
    {
        return stdx::locale_to_u8(T::Name()).append(u8": ").append(error_.DiagnosticInfo());
    }

private:
    static char const* Name() noexcept
    {
        return nullptr;
    }

private:
    ErrorType error_;
};

class _NormalExceptionImpl
    : public ExceptionBaseImpl<_NormalExceptionImpl>
{
public:
    _NormalExceptionImpl(base::Error error) noexcept
        : ExceptionBaseImpl{std::move(error)}
    {
    }

    static char const* Name() noexcept
    {
        return "Normal Exception";
    }
};

__declspec(noreturn) inline void ThrowException(Error error)
{
    throw _NormalExceptionImpl{std::move(error)};
}

} // namespace base
