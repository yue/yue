// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_EDIT_VIEW_H_
#define NATIVEUI_WIN_EDIT_VIEW_H_

#include <string>

#include "nativeui/win/subwin_view.h"

namespace nu {

class EditView : public SubwinView {
 public:
  EditView(View* delegate, DWORD styles);

  void SetPlainText();

  void SetText(const std::string& text);
  std::string GetText() const;

  void Redo();
  bool CanRedo() const;
  void Undo();
  bool CanUndo() const;

  void Cut();
  void Copy();
  void Paste();
  void SelectAll();

  bool is_editing() const { return is_editing_; }

 private:
  void LoadRichEdit();

  bool is_editing_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_EDIT_VIEW_H_
