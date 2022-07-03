#pragma once

#include "error.hpp"
#include <exception>
#include <string>

namespace base {

class Exception : virtual public std::exception
{
public:
    virtual std::error_code const& Code() const noexcept = 0;
    virtual std::u8string const& Message() const noexcept = 0;

    virtual void PushSourceLocation(SourceLocation const& location) noexcept = 0;
    virtual void PushInfo(ErrorInfo info) noexcept = 0;

    virtual std::u8string GetDiagnosticInfo(char8_t const* prefix = u8"\t < ", char8_t const* suffix = u8"\n") const = 0;

protected:
    Exception(char const* name = nullptr) noexcept
        : std::exception(name ? name : "Base Exception")
    {
    }
};

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

namespace internal {

template <class E>
struct enable_if_derived_from_exception
{
    using type = std::enable_if_t<std::is_base_of<Exception, E>::value, void>;
};

template <class E>
using enable_if_derived_from_exception_t = typename enable_if_derived_from_exception<E>::type;

template <class E>
inline E& PushInfo(E& ex, base::ErrorInfo const& info, enable_if_derived_from_exception_t<E>* = nullptr) noexcept
{
    ex.PushInfo(info);
    return ex;
}

template <class E>
inline void PushSourceLocation(E& ex, SourceLocation const& location, enable_if_derived_from_exception_t<E>* = nullptr)
{
    ex.PushSourceLocation(location);
}

} // namespace internal

} // namespace base
