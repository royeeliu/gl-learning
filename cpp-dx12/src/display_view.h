#pragma once

#include "framework.h"
#include "samples/sample_factory.h"

#include <memory>

class DisplayView final : public ATL::CWindowImpl<DisplayView>
{
public:
    DisplayView() = default;
    ~DisplayView() = default;

    // clang-format off
    BEGIN_MSG_MAP(DisplayView)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()
    // clang-format on

    void SetSampleFactory(samples::SampleFactory* factory);

private:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    SIZE GetClientSize();
    void CreateNewSample();

private:
    samples::SampleFactory* sample_factory_ = nullptr;
    std::unique_ptr<samples::SampleBase> sample_;
};
