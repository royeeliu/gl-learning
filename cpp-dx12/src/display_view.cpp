#include "display_view.h"


LRESULT DisplayView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0L;
}

LRESULT DisplayView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0L;
}

LRESULT DisplayView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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
