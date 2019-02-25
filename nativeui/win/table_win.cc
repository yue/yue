// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/table_win.h"

#include <commctrl.h>

#include <algorithm>
#include <string>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/table_model.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

TableImpl::TableImpl(Table* delegate)
    : SubwinView(delegate, WC_LISTVIEW,
                 LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_REPORT |
                 LVS_OWNERDATA | LVS_EDITLABELS | WS_CHILD | WS_VISIBLE) {
  set_focusable(true);
  ListView_SetExtendedListViewStyle(hwnd(),
                                    LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
}

TableImpl::~TableImpl() {
  if (image_list_)
    ImageList_Destroy(image_list_);
}

void TableImpl::AddColumnWithOptions(const base::string16& title,
                                     Table::ColumnOptions options) {
  LVCOLUMNA col = {0};
  col.mask = LVCF_TEXT;
  // The pszText is LPSTR even under Unicode build.
  col.pszText = const_cast<char*>(reinterpret_cast<const char*>(title.c_str()));

  // Insert.
  if (options.column == -1)
    options.column = GetColumnCount();
  ListView_InsertColumn(hwnd(), GetColumnCount(), &col);
  columns_.emplace_back(std::move(options));
  UpdateColumnsWidth(static_cast<Table*>(delegate())->GetModel());

  // Optimization in the custom draw handler.
  if (options.type == Table::ColumnType::Custom)
    has_custom_column_ = true;
}

int TableImpl::GetColumnCount() const {
  return static_cast<int>(columns_.size());
}

void TableImpl::UpdateColumnsWidth(TableModel* model) {
  int count = GetColumnCount();
  if (count == 0)
    return;

  // The AUTOSIZE style does not work for virtual list, we have to guess a
  // best width for each column.
  for (int i = 0; i < count - 1; ++i) {
    int width = columns_[i].width * scale_factor();
    if (width < 0) {  // autosize
      width = kDefaultColumnWidth * scale_factor();
      // If there is data in model, use the first cell's width.
      if (model && model->GetRowCount() > 0) {
        const base::Value* value = model->GetValue(i, 0);
        if (value && value->is_string()) {
          base::string16 text = base::UTF8ToUTF16(value->GetString());
          int text_width = ListView_GetStringWidth(hwnd(), text.c_str());
          // Add some padding.
          text_width += (i == 0 ? 7 : 14) * scale_factor();
          // Do not choose a too small width.
          width = std::max(width, text_width);
        }
      }
    }
    ListView_SetColumnWidth(hwnd(), i, width);
  }

  // Make the last column use USEHEADER style, which fills it to rest of the
  // list control.
  int width = columns_[count - 1].width * scale_factor();
  ListView_SetColumnWidth(hwnd(), count - 1,
                          width < 0 ? LVSCW_AUTOSIZE_USEHEADER : width);
}

void TableImpl::SetRowHeight(int height) {
  // ListView does not have a way to change row height, so we have to work out
  // our own way.
  if (!image_list_)
    image_list_ = ImageList_Create(1, height, 0, 0, 0);
  else
    ImageList_SetIconSize(image_list_, 1, height);
  ListView_SetImageList(hwnd(), image_list_, LVSIL_SMALL);
}

int TableImpl::GetRowHeight() const {
  if (image_list_) {
    int cx, cy;
    if (ImageList_GetIconSize(image_list_, &cx, &cy))
      return cy;
  }
  // Default row height be able to draw full text.
  return std::ceil(MeasureText(L"bp", font()).height());
}

void TableImpl::OnPaint(HDC dc) {
  // Block redrawing of leftmost item when editing sub item.
  if (edit_proc_) {
    RECT rc;
    ListView_GetItemRect(hwnd(), edit_row_, &rc, LVIR_LABEL);
    ::ValidateRect(hwnd(), &rc);
  }
  SetMsgHandled(false);
}

void TableImpl::OnWindowPosChanged(WINDOWPOS* pos) {
  SetMsgHandled(false);
  if (!window())
    return;

  // Resize the last column to fill the control.
  int count = GetColumnCount();
  if (count > 0 && columns_[count - 1].width == -1)
    ListView_SetColumnWidth(hwnd(), count - 1, LVSCW_AUTOSIZE_USEHEADER);
}

LRESULT TableImpl::OnNotify(int code, LPNMHDR pnmh) {
  switch (pnmh->code) {
    case LVN_GETDISPINFO: {
      auto* nm = reinterpret_cast<NMLVDISPINFO*>(pnmh);
      return OnGetDispInfo(nm, nm->item.iSubItem, nm->item.iItem);
    }
    case NM_CUSTOMDRAW: {
      auto* nm = reinterpret_cast<NMLVCUSTOMDRAW*>(pnmh);
      return OnCustomDraw(nm, nm->nmcd.dwItemSpec);
    }
    case LVN_BEGINLABELEDIT: {
      auto* nm = reinterpret_cast<NMLVDISPINFO*>(pnmh);
      return OnBeginEdit(nm, nm->item.iItem);
    }
    case LVN_ENDLABELEDIT: {
      auto* nm = reinterpret_cast<NMLVDISPINFO*>(pnmh);
      return OnEndEdit(nm, nm->item.iItem);
    }
    default:
      return 0;
  }
}

LRESULT TableImpl::OnGetDispInfo(NMLVDISPINFO* nm, int column, int row) {
  // When editing the sub item, hide the text.
  if (edit_proc_ && column == edit_column_ && row == edit_row_) {
    nm->item.pszText = TEXT("");
    return TRUE;
  }

  auto* model = static_cast<Table*>(delegate())->GetModel();
  if (!model)
    return 0;
  const base::Value* value = model->GetValue(column, row);
  if (!value)
    return 0;
  // Always set text regardless of cell type, for increased accessbility.
  if ((nm->item.mask & LVIF_TEXT) && value->is_string()) {
    text_cache_ = base::UTF8ToUTF16(value->GetString());
    nm->item.pszText = const_cast<wchar_t*>(text_cache_.c_str());
    return TRUE;
  }
  return 0;
}

LRESULT TableImpl::OnCustomDraw(NMLVCUSTOMDRAW* nm, int row) {
  if (!has_custom_column_)
    return 0;
  auto* model = static_cast<Table*>(delegate())->GetModel();
  if (!model)
    return 0;

  // Handle the post paint stage.
  switch (nm->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT:
      return CDRF_NOTIFYPOSTPAINT;
    case CDDS_ITEMPOSTPAINT:
      break;
    default:
      return CDRF_DODEFAULT;
  }

  // Draw custom type cells.
  for (int i = 0; i < GetColumnCount(); ++i) {
    const auto& options = columns_[i];
    if (options.type != Table::ColumnType::Custom || !options.on_draw)
      continue;
    const base::Value* value = model->GetValue(options.column, row);
    // Calculate the rect of each cell.
    RECT rc;
    ListView_GetSubItemRect(hwnd(), row, i, LVIR_BOUNDS, &rc);
    Rect rect(rc);
    // Reduce the cell area so the focus ring can show.
    int space = 1 * scale_factor();
    rect.Inset(space, space);
    // Get window size (needed by PainterWin).
    GetClientRect(hwnd(), &rc);
    // Draw.
    PainterWin painter(nm->nmcd.hdc, Rect(rc).size(), scale_factor());
    painter.TranslatePixel(rect.OffsetFromOrigin());
    painter.ClipRectPixel(Rect(rect.size()));
    options.on_draw(&painter,
                    RectF(ScaleSize(SizeF(rect.size()), 1.f / scale_factor())),
                    value ? value->Clone() : base::Value());
  }
  return CDRF_SKIPDEFAULT;
}

LRESULT TableImpl::OnBeginEdit(NMLVDISPINFO* nm, int row) {
  // Find out the column.
  LVHITTESTINFO hit = {0};
  ::GetCursorPos(&hit.pt);
  ::ScreenToClient(hwnd(), &hit.pt);
  ListView_SubItemHitTest(hwnd(), &hit);
  if (hit.iSubItem < 0 || hit.iSubItem >= GetColumnCount() || row != hit.iItem)
    return TRUE;
  int column = hit.iSubItem;

  // Only allow editing Edit type.
  if (columns_[column].type != Table::ColumnType::Edit)
    return TRUE;

  edit_row_ = row;
  edit_column_ = column;

  // Make the edit window work when editing sub items.
  if (column > 0) {
    // Subclass the edit window.
    edit_hwnd_ = ListView_GetEditControl(hwnd());
    SetWindowUserData(edit_hwnd_, this);
    edit_proc_ = SetWindowProc(edit_hwnd_, &EditWndProc);
    // Set pos to cell.
    RECT rc;
    ListView_GetSubItemRect(hwnd(), row, column, LVIR_LABEL, &rc);
    edit_pos_ = Rect(rc).origin();
    // Update text in edit window.
    auto* model = static_cast<Table*>(delegate())->GetModel();
    if (model) {
      const base::Value* value = model->GetValue(column, row);
      if (value && value->is_string()) {
        base::string16 text16 = base::UTF8ToUTF16(value->GetString());
        ::SetWindowTextW(edit_hwnd_, text16.c_str());
      }
    }
  }
  return FALSE;
}

LRESULT TableImpl::OnEndEdit(NMLVDISPINFO* nm, int row) {
  // Notify the result.
  DCHECK_EQ(row, edit_row_);
  if (nm->item.pszText != nullptr) {
    auto* model = static_cast<Table*>(delegate())->GetModel();
    if (model)
      model->SetValue(edit_column_, edit_row_,
                      base::Value(base::UTF16ToUTF8(nm->item.pszText)));
  }

  edit_row_ = -1;
  edit_column_ = -1;

  if (edit_proc_) {
    // Revert edit window subclass.
    SetWindowProc(edit_hwnd_, edit_proc_);
    edit_proc_ = nullptr;
    // Do nothing as ListView thinks we are editing the first column.
    return FALSE;
  }
  return TRUE;
}

// static
LRESULT TableImpl::EditWndProc(HWND hwnd,
                               UINT message,
                               WPARAM w_param,
                               LPARAM l_param) {
  auto* self = reinterpret_cast<TableImpl*>(GetWindowUserData(hwnd));
  // Force moving the edit window.
  if (message == WM_WINDOWPOSCHANGING) {
    auto* pos = reinterpret_cast<LPWINDOWPOS>(l_param);
    pos->x = self->edit_pos_.x();
    pos->y = self->edit_pos_.y();
  }
  return CallWindowProc(self->edit_proc_, hwnd, message, w_param, l_param);
}

///////////////////////////////////////////////////////////////////////////////
// Public Table API implementation.

NativeView Table::PlatformCreate() {
  return new TableImpl(this);
}

void Table::PlatformDestroy() {
}

void Table::PlatformSetModel(TableModel* model) {
  auto* table = static_cast<TableImpl*>(GetNative());
  if (GetModel()) {
    // Deselect everything.
    ListView_SetItemState(table->hwnd(), -1, LVIF_STATE, LVIS_SELECTED);
    // Scroll back to top, otherwise listview will have rendering bugs.
    ListView_EnsureVisible(table->hwnd(), 0, FALSE);
  }
  // Update row count.
  ListView_SetItemCountEx(table->hwnd(), model ? model->GetRowCount() : 0, 0);
  if (model) {
    // Listview does not update column width automatically, recalculate width
    // after changing model.
    table->UpdateColumnsWidth(model);
    // After updating column width, we have to set SetItemCount again to force
    // listview to update scrollbar.
    ListView_SetItemCountEx(table->hwnd(), model->GetRowCount(), 0);
  }
}

void Table::AddColumnWithOptions(const std::string& title,
                                 const ColumnOptions& options) {
  auto* table = static_cast<TableImpl*>(GetNative());
  table->AddColumnWithOptions(base::UTF8ToUTF16(title), options);
}

int Table::GetColumnCount() const {
  auto* table = static_cast<TableImpl*>(GetNative());
  return table->GetColumnCount();
}

void Table::SetColumnsVisible(bool visible) {
  auto* table = static_cast<TableImpl*>(GetNative());
  LONG styles = ::GetWindowLong(table->hwnd(), GWL_STYLE);
  if (!visible)
    styles |= LVS_NOCOLUMNHEADER;
  else
    styles &= ~LVS_NOCOLUMNHEADER;
  ::SetWindowLong(table->hwnd(), GWL_STYLE, styles);
}

bool Table::IsColumnsVisible() const {
  auto* table = static_cast<TableImpl*>(GetNative());
  return !(::GetWindowLong(table->hwnd(), GWL_STYLE) & LVS_NOCOLUMNHEADER);
}

void Table::SetRowHeight(float height) {
  auto* table = static_cast<TableImpl*>(GetNative());
  table->SetRowHeight(std::ceil(height * table->scale_factor()));
  // Update scrollbar after changing row height.
  if (GetModel())
    ListView_SetItemCountEx(table->hwnd(), GetModel()->GetRowCount(), 0);
}

float Table::GetRowHeight() const {
  auto* table = static_cast<TableImpl*>(GetNative());
  return table->GetRowHeight() / table->scale_factor();
}

void Table::SelectRow(int row) {
  auto* table = static_cast<TableImpl*>(GetNative());
  ListView_SetItemState(table->hwnd(), row, LVIS_SELECTED, LVIS_SELECTED);
}

int Table::GetSelectedRow() const {
  auto* table = static_cast<TableImpl*>(GetNative());
  return ListView_GetNextItem(table->hwnd(), -1, LVNI_SELECTED);
}

void Table::NotifyRowInsertion(uint32_t row) {
  auto* table = static_cast<TableImpl*>(GetNative());
  ListView_SetItemCountEx(table->hwnd(), GetModel()->GetRowCount(),
                          LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}

void Table::NotifyRowDeletion(uint32_t row) {
  auto* table = static_cast<TableImpl*>(GetNative());
  ListView_SetItemCountEx(table->hwnd(), GetModel()->GetRowCount(),
                          LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}

void Table::NotifyValueChange(uint32_t column, uint32_t row) {
  auto* table = static_cast<TableImpl*>(GetNative());
  ListView_Update(table->hwnd(), row);
}

}  // namespace nu
