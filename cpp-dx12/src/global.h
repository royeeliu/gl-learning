#pragma once

#include "framework.h"
#include "base/error.hpp"

namespace global {

extern CAppModule app_module;

void ShowErrorInfo(base::Error const& error);

} // namespace global

