#include "display_view.h"
#include "global.h"

#include <stdexcept>

LRESULT DisplayView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    sample_ = std::make_unique<samples::HelloWindow>(m_hWnd, [](auto const& error) { global::ShowErrorInfo(error); });
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
    return 0L;
}
