// dx12learning.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "framework.h"
#include "main_frame.h"

#include <iostream>

namespace global {

CAppModule app_module;

} // namespace global

int main()
{
    HRESULT hr = ::CoInitialize(nullptr);
    ATLASSERT(SUCCEEDED(hr));

    AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES);

    auto& app_module = global::app_module;
    hr = app_module.Init(nullptr, static_cast<HINSTANCE>(::GetModuleHandle(nullptr)));
    ATLASSERT(SUCCEEDED(hr));

    MSG msg{};
    // force message queue to be created
    ::PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    CMessageLoop message_loop;
    app_module.AddMessageLoop(&message_loop);

    MainFrame main_frame;

    RECT rect{0, 0, 1280, 720};
    if (main_frame.CreateEx(/*hWndParent*/nullptr, &rect) == nullptr)
    {
        ATLTRACE(_T("Frame window creation failed!\n"));
        return 0;
    }

    main_frame.CenterWindow();
    main_frame.ShowWindow(SW_SHOW);
    ::SetForegroundWindow(main_frame); // Win95 needs this

    int exit_code = message_loop.Run();

    app_module.RemoveMessageLoop();

    app_module.Term();
    ::CoUninitialize();

    return exit_code;
}
