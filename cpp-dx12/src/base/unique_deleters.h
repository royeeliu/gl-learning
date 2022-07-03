#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>

#include <memory>

namespace base {

struct handle_delete
{ 
    void operator()(void* ptr) const noexcept
    {                                        
        ::CloseHandle(ptr);
    }
};

} // namespace base
