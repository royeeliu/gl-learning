#include "display_view.h"
#include "global.h"
#include "samples/hello_triangle.h"
#include "samples/hello_window.h"

LRESULT DisplayView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //sample_ = std::make_unique<samples::HelloTriangle>(m_hWnd, [](auto const& error) { global::ShowErrorInfo(error); });
    return 0L;
}

LRESULT DisplayView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0L;
}

LRESULT DisplayView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (sample_ && sample_->Render())
    {
        return 0L;
    }

    CPaintDC dc(m_hWnd);
    RECT rect{};
    GetClientRect(&rect);
    dc.FillSolidRect(&rect, RGB(128, 128, 128));

    return 0L;
}

LRESULT DisplayView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return 1L;
}

LRESULT DisplayView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    uint32_t width = LOWORD(lParam);  // width of client area
    uint32_t height = HIWORD(lParam); // height of client area
    if ((width > 0) && (height > 0) && !sample_)
    {
        sample_ =
            std::make_unique<samples::HelloTriangle>(m_hWnd, [](auto const& error) { global::ShowErrorInfo(error); });
    }
    return 0L;
}
