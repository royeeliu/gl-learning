#pragma once

#include "errors.hpp"
#include "exception.hpp"
#include "stdx/strings.hpp"

#include <format>

namespace base {

class com_error_category : public std::error_category
{
public:
    com_error_category() noexcept
        : error_category()
    {
    }

    virtual const char* name() const noexcept override
    {
        return "windows com error";
    }

    virtual std::string message(int code) const override
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<uint32_t>(code));
        return std::string(s_str);
    }

    static std::error_category const& get_category()
    {
        static com_error_category s_instance;
        return s_instance;
    }
};

inline std::error_code MakeComErrorCode(HRESULT code)
{
    return std::error_code(static_cast<int>(code), com_error_category::get_category());
}

class ComError : public Error
{
public:
    explicit ComError(HRESULT hr, char8_t const* message = nullptr)
        : Error{MakeComErrorCode(hr), message}
    {
    }

    ComError(HRESULT hr, std::u8string message) noexcept
        : Error{MakeComErrorCode(hr), message}
    {
    }

    ~ComError() noexcept override {}

    ComError(ComError&&) noexcept = default;
    ComError& operator=(ComError&&) noexcept = default;

    HRESULT HResult() const noexcept
    {
        return static_cast<HRESULT>(Code().value());
    }
};

inline ComError MakeComError(HRESULT hr, char const* api)
{
    return ComError{hr, stdx::locale_to_u8(std::format("call {0} return 0x{1:8x}", api, hr))};
} 

class ComException : virtual public Exception
{
public:
    virtual HRESULT HResult() const noexcept = 0;

protected:
    ComException(char const* name) noexcept
        : Exception{name ? name : "COM Exception"}
    {
    }
};

template <
    class T, class TError = ComError, class TBase = ComException,
    class = typename std::enable_if<
        std::is_base_of<ComError, TError>::value && std::is_base_of<ComException, TBase>::value, void>::type>
class ComExceptionBaseImpl : public ExceptionBaseImpl<T, TError, TBase>
{
public:
    using ErrorType = TError;
    using BaseClass = ExceptionBaseImpl<T, TError, TBase>;

    explicit ComExceptionBaseImpl(ErrorType error) noexcept
        : BaseClass{std::move(error)}
    {
    }

    HRESULT HResult() const noexcept override 
    {
        return __super::Error().HResult();
    }
};

class _ComExceptionImpl : public ComExceptionBaseImpl<_ComExceptionImpl>
{
    using BaseClass = ComExceptionBaseImpl<_ComExceptionImpl>;

public:
    _ComExceptionImpl(ComError error)
        : BaseClass{std::move(error)}
    {
    }
};

__declspec(noreturn) inline void ThrowException(ComError error)
{
    throw _ComExceptionImpl{std::move(error)};
}

} // namespace base
