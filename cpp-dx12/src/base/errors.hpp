#pragma once

#include "stdx/strings.hpp"

#include <format>
#include <iostream>
#include <system_error>
#include <vector>

namespace base {

struct ErrorInfo
{
    std::u8string name;
    std::u8string content;
};

struct SourceLocation
{
    char8_t const* file = nullptr;
    int line = 0;
    char8_t const* function = nullptr;
};

template <class T>
ErrorInfo MakeErrorInfo(char8_t const* name, T content) noexcept
{
    try
    {
        return {std::u8string{name ? name : u8""}, stdx::locale_to_u8(std::to_string(content))};
    }
    catch (std::exception&)
    {
        return ErrorInfo{};
    }
}

inline ErrorInfo MakeErrorInfo(char8_t const* name, std::string content)
{
    try
    {
        return {std::u8string{name ? name : u8""}, stdx::locale_to_u8(content)};
    }
    catch (std::exception&)
    {
        return ErrorInfo{};
    }
}

inline ErrorInfo MakeErrorInfo(char8_t const* name, char const* content)
{
    try
    {
        return {std::u8string{name ? name : u8""}, stdx::locale_to_u8(content)};
    }
    catch (std::exception&)
    {
        return ErrorInfo{};
    }
}

inline ErrorInfo MakeErrorInfo(char8_t const* name, char* content)
{
    try
    {
        return {std::u8string{name ? name : u8""}, stdx::locale_to_u8(content)};
    }
    catch (std::exception&)
    {
        return ErrorInfo{};
    }
}

class Error
{
public:
    Error(std::error_code const& ec, std::u8string message) noexcept
        : code_{ec}
        , message_{std::move(message)}
    {
    }

    explicit Error(std::error_code const& ec, char8_t const* message = nullptr) noexcept
        : Error{ec, MakeMessage(message)}
    {
    }

    virtual ~Error() noexcept {}

    Error(Error&&) noexcept = default;
    Error& operator=(Error&&) noexcept = default;

    std::error_code const& Code() const noexcept
    {
        return code_;
    }

    std::u8string const& Message() const noexcept
    {
        return message_;
    }

    std::u8string DiagnosticInfo(char8_t const* prefix = u8"\t < ", char8_t const* suffix = u8"\n") const
    {
        std::u8string info;
        info.append(message_)
            .append(suffix)
            .append(prefix)
            .append(u8"code: ")
            .append(stdx::locale_to_u8(std::format("{}", code_.value())))
            .append(suffix)
            .append(prefix)
            .append(u8"description: ")
            .append(stdx::locale_to_u8(code_.message()))
            .append(suffix);
        for (auto const& item : infos_)
        {
            info.append(prefix).append(item.name).append(u8": ").append(suffix);
        }
        for (auto const& item : source_locations_)
        {
            info.append(prefix)
                .append(item.file ? item.file : u8"unknown file")
                .append(stdx::locale_to_u8(std::format("({}): ", item.line)))
                .append(item.function ? item.function : u8"unknown function")
                .append(suffix);
        }
        return info;
    }

    void PushSourceLocation(SourceLocation const& location) noexcept
    {
        try
        {
            source_locations_.push_back(location);
        }
        catch (std::exception&)
        {
        }
    }

    void PushInfo(ErrorInfo info) noexcept
    {
        try
        {
            infos_.emplace_back(std::move(info));
        }
        catch (std::exception&)
        {
        }
    }

private:
    static std::u8string MakeMessage(char8_t const* message) noexcept
    {
        try
        {
            return std::u8string{message ? message : u8"Unknown error"};
        }
        catch (std::exception&)
        {
            return std::u8string{};
        }
    }

private:
    std::u8string message_;
    std::error_code code_;
    std::vector<ErrorInfo> infos_;
    std::vector<SourceLocation> source_locations_;
};

class Exception : virtual public std::exception
{
public:
    virtual std::error_code const& Code() const noexcept = 0;
    virtual std::u8string const& Message() const noexcept = 0;

    virtual void PushSourceLocation(SourceLocation const& location) noexcept = 0;
    virtual void PushInfo(ErrorInfo info) noexcept = 0;

    virtual std::u8string GetDiagnosticInfo(
        char8_t const* prefix = u8"\t < ", char8_t const* suffix = u8"\n") const = 0;

protected:
    Exception(char const* name = nullptr) noexcept
        : std::exception(name ? name : "Base Exception")
    {
    }
};

namespace internal {

inline void PushInfo(Error& error, base::ErrorInfo const& info) noexcept
{
    error.PushInfo(info);
}

inline void PushInfo(Exception& ex, base::ErrorInfo const& info) noexcept
{
    ex.PushInfo(info);
}

inline void PushInfo(std::ostream& os, base::ErrorInfo const& info) noexcept
{
    os << "{" << stdx::u8_to_locale(info.name) << ": " << stdx::u8_to_locale(info.content) << "}";
}

inline void PushSourceLocation(Error& error, SourceLocation const& location)
{
    error.PushSourceLocation(location);
}

inline void PushSourceLocation(Exception& ex, SourceLocation const& location)
{
    ex.PushSourceLocation(location);
}

inline void PushSourceLocation(std::ostream& os, SourceLocation const& location)
{
    os << "{" << stdx::u8_to_locale(location.file) << "(" << location.line
       << "): " << stdx::u8_to_locale(location.function) << "}";
}

} // namespace internal

} // namespace base

template <class E>
inline E& operator<<(E&& e, base::ErrorInfo const& info) noexcept
{
    base::internal::PushInfo(e, info);
    return e;
}

template <class E>
inline E& operator<<(E&& e, base::SourceLocation const& location) noexcept
{
    base::internal::PushSourceLocation(e, location);
    return e;
}

#define _BASE_U8_(s) u8##s
#define _BASE_U8(s) _BASE_U8_(s)

#define MAKE_SOURCE_LOCATION()                                                                                         \
    base::SourceLocation                                                                                               \
    {                                                                                                                  \
        _BASE_U8(__FILE__), __LINE__, _BASE_U8(__FUNCTION__)                                                           \
    }
