#include "global.h"
#include "stdx/strings.h"

#include <iostream>

namespace global {

void ShowErrorInfo(base::Error const& error)
{
    std::cout << "[ERROR]:" << stdx::u8_to_locale(error.DiagnosticInfo());
}

} // namespace global

