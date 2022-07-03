#pragma once

#include "errors.hpp"
#include <variant>

namespace base {

template <class T, class E>
struct Result
{
    std::variant<T, E> value;

    bool IsSucceeded() const
    {
        return value.index() == 0;
    }

    bool IsFailed() const
    {
        return value.index() != 0;
    }

    operator bool() const {
        return IsSucceeded();
    }
};

} // namespace base
