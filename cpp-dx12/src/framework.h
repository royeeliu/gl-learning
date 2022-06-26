#pragma once

#include "targetver.h"

// 从 Windows 头文件中排除极少使用的内容
#define WIN32_LEAN_AND_MEAN             

// Windows Header
#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>

// WTL Header
#pragma warning(push)
// Disable IntelliSense warning
#pragma warning(disable : 26110)
#pragma warning(disable : 26451)
#pragma warning(disable : 26454)
#pragma warning(disable : 6387)
#pragma warning(disable : 6001)
#include "wtl/atlapp.h"
#include "wtl/atlframe.h"
#include "wtl/atlsplit.h"
#include "wtl/atlmisc.h"
#include "wtl/atlctrls.h"
#include "wtl/atlctrlw.h"
#include "wtl/atlctrlx.h"
#pragma warning(pop)

#if defined _M_IX86
#pragma comment(                                                                                                       \
    linker,                                                                                                            \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(                                                                                                       \
    linker,                                                                                                            \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(                                                                                                       \
    linker,                                                                                                            \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(                                                                                                       \
    linker,                                                                                                            \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
