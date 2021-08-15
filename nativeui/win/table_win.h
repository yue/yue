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
  ~TableImpl() final;

  static const int kDefaultColumnWidth = 50;

  void AddColumnWithOptions(const std::wstring& title,
                            Table::ColumnOptions options);
  int GetColumnCount() const;
  void UpdateColumnsWidth(TableModel* model);
  void SetRowHeight(int height);
  int GetRowHeight() const;

 protected:
  CR_BEGIN_MSG_MAP_EX(TableImpl, SubwinView)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
  CR_END_MSG_MAP()

  // SubwinView:
  LRESULT OnNotify(int code, LPNMHDR pnmh) override;

 private:
  void OnPaint(HDC dc);
  void OnWindowPosChanged(WINDOWPOS* pos);

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

  // The ImageList hack used for changing row height.
  HIMAGELIST image_list_ = NULL;

  // The pszText must be valid when the message is sent, so we have to keep
  // a cache to avoid returning a pointer to temporary memory.
  std::wstring text_cache_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_TABLE_WIN_H_
