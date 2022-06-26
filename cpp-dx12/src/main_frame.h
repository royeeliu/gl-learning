#pragma once

#include "framework.h"
#include "display_view.h"

#include <string>
#include <vector>

#pragma warning(push)
// Disable IntelliSense warning
#pragma warning(disable : 26454) // NOTIFY_CODE_HANDLER 宏展开代码算术溢出

class MainFrame final
    : public CFrameWindowImpl<MainFrame>
    , public CUpdateUI<MainFrame>
    , public CMessageFilter
    , public CIdleHandler
{
public:
    MainFrame() = default;
    ~MainFrame() = default;

    // clang-format off
    BEGIN_MSG_MAP(MainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTVSelChanged)

        CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<MainFrame>)
    END_MSG_MAP()

    BEGIN_UPDATE_UI_MAP(MainFrame)
    END_UPDATE_UI_MAP()
    // clang-format on

private:
    // override CMessageFilter method
    BOOL PreTranslateMessage(MSG* msg) override;

    // override CIdleHandler method
    BOOL OnIdle() override;

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTVSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

private:
    struct TVItem
    {
        std::wstring name;
        std::vector<TVItem> children;
    };

    void InitViews();
    void FillTreeView(const std::vector<TVItem>& items, HTREEITEM parent);
    void UpdateTitle(const wchar_t* suffix);

    static std::vector<TVItem> CreateItemTree();

private:
    CSplitterWindow splitter_view_;
    CTreeViewCtrlEx tree_view_;
    DisplayView display_view_;

    bool first_idle_ = true;
};

#pragma warning(pop)
