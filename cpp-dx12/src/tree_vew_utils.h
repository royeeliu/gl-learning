#pragma once

#include "framework.h"

namespace tree_view_utils {

static void ExpandRecursive(CTreeItem& item)
{
    if (item.IsNull() || !item.HasChildren())
    {
        return;
    }

    item.Expand();
    CTreeItem child = item.GetChild();
    while (!child.IsNull())
    {
        ExpandRecursive(child);
        child = child.GetNext(TVGN_NEXT);
    }
}

inline void ExpantAll(CTreeViewCtrlEx& tree_view)
{
    CTreeItem item = tree_view.GetRootItem();
    while (!item.IsNull())
    {
        ExpandRecursive(item);
        item = item.GetNext(TVGN_NEXT);
    }
}

static CTreeItem FindFirstLeaf(const CTreeItem& item) {
    if (item.IsNull() || !item.HasChildren())
    {
        return item;
    }
    return FindFirstLeaf(item.GetChild());
}

} // namespace tree_view_utils
