#pragma once

#if !defined(_INC_WINDOWS)
#error Need include windows.h first.
#endif

#include <string>

namespace stdx {
namespace impl {

inline std::string wide_to_locale_string(const wchar_t* wstr, int size = -1)
{
    if (!wstr || wstr[0] == L'\0')
    {
        return "";
    }

    int count = ::WideCharToMultiByte(CP_ACP, 0, wstr, size, nullptr, 0, nullptr, nullptr);
    if (count == 0)
    {
        return "";
    }

    std::string str(static_cast<std::size_t>(count), '\0');
    ::WideCharToMultiByte(CP_ACP, 0, wstr, size, &str[0], count, nullptr, nullptr);
    if (size == -1 && str.back() == '\0')
    {
        str.pop_back();
    }
    return str;
}

inline std::u8string wide_to_utf8_string(const wchar_t* wstr, int size = -1)
{
    if (!wstr || wstr[0] == L'\0')
    {
        return u8"";
    }

    int count = ::WideCharToMultiByte(CP_UTF8, 0, wstr, size, nullptr, 0, nullptr, nullptr);
    if (count == 0)
    {
        return u8"";
    }

    std::u8string str(static_cast<std::size_t>(count), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wstr, size, (char*)&str[0], count, nullptr, nullptr);
    if (size == -1 && str.back() == '\0')
    {
        str.pop_back();
    }
    return str;
}

inline std::wstring locale_to_wide_string(const char* str, int size = -1)
{
    if (!str || str[0] == '\0')
    {
        return L"";
    }

    int count = ::MultiByteToWideChar(CP_ACP, 0, str, size, nullptr, 0);
    if (count == 0)
    {
        return L"";
    }

    std::wstring wstr(count, L'\0');
    ::MultiByteToWideChar(CP_ACP, 0, str, size, &wstr[0], count);
    if (size == -1 && wstr.back() == L'\0')
    {
        wstr.pop_back();
    }
    return wstr;
}

inline std::wstring utf8_to_wide_string(const char8_t* str, int size = -1)
{
    if (!str || str[0] == '\0')
    {
        return L"";
    }

    int count = ::MultiByteToWideChar(CP_UTF8, 0, (const char*)str, size, nullptr, 0);
    if (count == 0)
    {
        return L"";
    }

    std::wstring wstr(count, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, (const char*)str, size, &wstr[0], count);
    if (size == -1 && wstr.back() == L'\0')
    {
        wstr.pop_back();
    }
    return wstr;
}

} // namespace impl

inline std::u8string wide_to_u8(const std::wstring& wstr)
{
    return impl::wide_to_utf8_string(wstr.c_str(), (int)wstr.size());
}

inline std::u8string wide_to_u8(const wchar_t* wstr)
{
    return impl::wide_to_utf8_string(wstr);
}

inline std::u8string wide_to_u8(const wchar_t* wstr, std::size_t size)
{
    return impl::wide_to_utf8_string(wstr, (int)size);
}

inline std::string wide_to_locale(const std::wstring& wstr)
{
    return impl::wide_to_locale_string(wstr.c_str(), (int)wstr.size());
}

inline std::string wide_to_locale(const wchar_t* wstr)
{
    return impl::wide_to_locale_string(wstr);
}

inline std::string wide_to_locale(const wchar_t* wstr, std::size_t size)
{
    return impl::wide_to_locale_string(wstr, (int)size);
}

inline std::wstring locale_to_wide(const std::string& str)
{
    return impl::locale_to_wide_string(str.c_str(), (int)str.size());
}

inline std::wstring locale_to_wide(const char* str)
{
    return impl::locale_to_wide_string(str);
}

inline std::wstring locale_to_wide(const char* str, std::size_t size)
{
    return impl::locale_to_wide_string(str, (int)size);
}

inline std::wstring u8_to_wide(const std::u8string& str)
{
    return impl::utf8_to_wide_string(str.c_str(), (int)str.size());
}

inline std::wstring u8_to_wide(const char8_t* str)
{
    return impl::utf8_to_wide_string(str);
}

inline std::wstring u8_to_wide(const char8_t* str, std::size_t size)
{
    return impl::utf8_to_wide_string(str, (int)size);
}

inline std::u8string locale_to_u8(const std::string& str)
{
    return wide_to_u8(locale_to_wide(str));
}

inline std::u8string locale_to_u8(const char* str)
{
    return wide_to_u8(locale_to_wide(str));
}

inline std::u8string locale_to_u8(const char* str, std::size_t size)
{
    return wide_to_u8(locale_to_wide(str, size));
}

inline std::string u8_to_locale(const std::u8string& str)
{
    return wide_to_locale(u8_to_wide(str));
}

inline std::string u8_to_locale(const char8_t* str)
{
    return wide_to_locale(u8_to_wide(str));
}

inline std::string u8_to_locale(const char8_t* str, std::size_t size)
{
    return wide_to_locale(u8_to_wide(str, size));
}


} // namespace stdx


