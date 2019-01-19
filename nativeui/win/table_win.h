// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_TABLE_WIN_H_
#define NATIVEUI_WIN_TABLE_WIN_H_

#include <vector>

#include "nativeui/table.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

class TableImpl : public SubwinView {
 public:
  explicit TableImpl(Table* delegate);

  static const int kDefaultColumnWidth = 50;

  void AddColumnWithOptions(const base::string16& title,
                            Table::ColumnOptions options);
  int GetColumnCount() const;
  void UpdateColumnsWidth(TableModel* model);

 protected:
  // SubwinView:
  LRESULT OnNotify(int code, LPNMHDR pnmh) override;

  CR_BEGIN_MSG_MAP_EX(TableImpl, SubwinView)
    CR_MSG_WM_PAINT(OnPaint)
  CR_END_MSG_MAP()

 private:
  void OnPaint(HDC dc);
  LRESULT OnGetDispInfo(NMLVDISPINFO* nm, int column, int row);
  LRESULT OnCustomDraw(NMLVCUSTOMDRAW* nm, int row);
  LRESULT OnBeginEdit(NMLVDISPINFO* nm, int row);
  LRESULT OnEndEdit(NMLVDISPINFO* nm, int row);

  static LRESULT CALLBACK EditWndProc(HWND hwnd,
                                      UINT message,
                                      WPARAM w_param,
                                      LPARAM l_param);

  std::vector<Table::ColumnOptions> columns_;

  // Whether there are custom drawing cells.
  bool has_custom_column_ = false;

  // The handle to the edit window.
  HWND edit_hwnd_ = NULL;
  WNDPROC edit_proc_ = nullptr;

  // Set when editing items.
  int edit_row_ = -1;
  int edit_column_ = -1;
  Point edit_pos_;

  // The pszText must be valid when the message is sent, so we have to keep
  // a cache to avoid returning a pointer to temporary memory.
  base::string16 text_cache_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_TABLE_WIN_H_
