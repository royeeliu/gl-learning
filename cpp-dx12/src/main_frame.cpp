#include "main_frame.h"
#include "config.h"
#include "global.h"
#include "stdx/strings.h"
#include "tree_vew_utils.h"

BOOL MainFrame::PreTranslateMessage(MSG* msg)
{
    return CFrameWindowImpl<MainFrame>::PreTranslateMessage(msg);
}

BOOL MainFrame::OnIdle()
{
    if (first_idle_)
    {
        first_idle_ = false;

        // Hourglass on
        CWaitCursor wait;

        FillTreeView(CreateItemTree(), TVI_ROOT);
        tree_view_utils::ExpantAll(tree_view_);
        tree_view_.SelectItem(tree_view_utils::FindFirstLeaf(tree_view_.GetRootItem()));
        tree_view_.SetFocus();
    }
    return FALSE;
}

void MainFrame::InitViews()
{
}

void MainFrame::FillTreeView(const std::vector<TVItem>& items, HTREEITEM parent)
{
    TVITEM tvi = {0};           
    TVINSERTSTRUCT tvins = {0};  
    HTREEITEM prev_item = nullptr;    

    for (const auto& item : items)
    {
        tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        tvi.pszText = const_cast<LPWSTR>(item.name.c_str());
        tvi.cchTextMax = static_cast<int>(item.name.length());
        tvi.lParam = static_cast<LPARAM>(0);

        if (!item.children.empty())
        {
            tvi.cChildren = static_cast<int>(item.children.size());
            tvi.mask |= TVIF_CHILDREN;
        }

        tvins.item = tvi;
        tvins.hInsertAfter = prev_item;
        tvins.hParent = parent;

        auto current_item = TreeView_InsertItem(tree_view_.m_hWnd, &tvins);
        FillTreeView(item.children, current_item);
        prev_item = current_item;
    }
}

void MainFrame::UpdateTitle(const wchar_t* suffix)
{
    std::wstring title = stdx::u8_to_wide(config::AppName);
    if (suffix && suffix[0] != L'\0')
    {
        title.append(L" - ").append(suffix);
    }
    SetWindowTextW(title.c_str());
}

std::vector<MainFrame::TVItem> MainFrame::CreateItemTree() {
    TVItem item1{L"item 1"};
    item1.children.emplace_back(L"leaf 1_1");
    item1.children.emplace_back(L"leaf 1_2");
    item1.children.emplace_back(L"leaf 1_3");
    TVItem item2{L"item 2"};
    TVItem item3{L"item 3"};
    item3.children.emplace_back(L"leaf 3_1");
    item3.children.emplace_back(L"item 3_2");
    item3.children.back().children.emplace_back(L"leaf 3_2_1");

    std::vector<TVItem> item_tree;
    item_tree.emplace_back(std::move(item1));
    item_tree.emplace_back(std::move(item2));
    item_tree.emplace_back(std::move(item3));

    return item_tree;
}

LRESULT MainFrame::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
    UpdateTitle(/*suffix*/ nullptr);
    CreateSimpleStatusBar();

    m_hWndClient = splitter_view_.Create(
        m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    tree_view_.Create(
        splitter_view_, rcDefault, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |
            TVS_SHOWSELALWAYS,
        WS_EX_CLIENTEDGE);

    display_view_.Create(
        splitter_view_, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0);

    InitViews();
    UpdateLayout();

    splitter_view_.SetSplitterPanes(tree_view_, display_view_);

    RECT rect{};
    GetClientRect(&rect);
    splitter_view_.SetSplitterPos((rect.right - rect.left) / 4);

    CMessageLoop* loop = global::app_module.GetMessageLoop();
    loop->AddMessageFilter(this);
    loop->AddIdleHandler(this);

    return 0L;
}

LRESULT MainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CMessageLoop* loop = global::app_module.GetMessageLoop();
    if (loop)
    {
        loop->RemoveMessageFilter(this);
        loop->RemoveIdleHandler(this);
    }
    ::PostQuitMessage(0);
    return 0L;
}

LRESULT MainFrame::OnTVSelChanged(int, LPNMHDR pnmh, BOOL&)
{
    LPNMTREEVIEW lptv = (LPNMTREEVIEW)pnmh;
    CTreeViewCtrlEx tree_view(lptv->hdr.hwndFrom);

    CString text;
    tree_view.GetItemText(lptv->itemNew.hItem, text);
    UpdateTitle(text);

    return 0L;
}

