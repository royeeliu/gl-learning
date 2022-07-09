#include "display_view.h"
#include "base/macros.hpp"
#include "global.h"

void DisplayView::SetSampleFactory(samples::SampleFactory* factory) {
    if (sample_factory_ != factory)
    {
        sample_factory_ = factory;
        SIZE size = GetClientSize();
        if ((sample_factory_ != nullptr) && (size.cx > 0) && (size.cy > 0))
        {
            CreateNewSample();
        }
    }
}

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
    if ((width > 0) && (height > 0) && !sample_ && sample_factory_)
    {
        CreateNewSample();
    }
    return 0L;
}

SIZE DisplayView::GetClientSize() {
    RECT rect{};
    GetClientRect(&rect);
    return {rect.right - rect.left, rect.bottom - rect.top};
}

void DisplayView::CreateNewSample() {
    ENSURE(sample_factory_ != nullptr);
    sample_.reset();
    sample_ = sample_factory_->Create(m_hWnd, [](auto const& error) { global::ShowErrorInfo(error); });
    InvalidateRect(nullptr);
}
