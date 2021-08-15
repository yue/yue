// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/edit_view.h"

#include <richedit.h>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/text_edit.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

EditView::EditView(View* delegate, DWORD styles)
    : SubwinView((LoadRichEdit(), delegate),  // load dll before constructor
                 MSFTEDIT_CLASS,
                 WS_CHILD | WS_VISIBLE | styles,
                 WS_EX_CLIENTEDGE) {
  set_focusable(true);
  ::SendMessage(hwnd(), EM_SETEVENTMASK, 0L, ENM_CHANGE | ENM_REQUESTRESIZE);
}

void EditView::SetPlainText() {
  ::SendMessage(hwnd(), EM_SETTEXTMODE, TM_PLAINTEXT, 0L);
}

void EditView::SetText(const std::string& text) {
  is_editing_ = true;
  std::wstring text16 = base::UTF8ToWide(text);
  ::SetWindowTextW(hwnd(), text16.c_str());
  is_editing_ = false;
  // Scroll to end after setting text, this follows the behavior on other
  // platforms.
  ::SendMessage(hwnd(), EM_SETSEL, text16.size(), text16.size());
}

std::string EditView::GetText() const {
  return base::WideToUTF8(GetWindowString(hwnd()));
}

void EditView::Redo() {
  ::SendMessage(hwnd(), EM_REDO, 0, 0L);
}

bool EditView::CanRedo() const {
  return ::SendMessage(hwnd(), EM_CANREDO, 0, 0L) != 0;
}

void EditView::Undo() {
  ::SendMessage(hwnd(), EM_UNDO, 0, 0L);
}

bool EditView::CanUndo() const {
  return ::SendMessage(hwnd(), EM_CANUNDO, 0, 0L) != 0;
}

void EditView::Cut() {
  ::SendMessage(hwnd(), WM_CUT, 0, 0L);
}

void EditView::Copy() {
  ::SendMessage(hwnd(), WM_COPY, 0, 0L);
}

void EditView::Paste() {
  ::SendMessage(hwnd(), WM_PASTE, 0, 0L);
}

void EditView::SelectAll() {
  ::SendMessage(hwnd(), EM_SETSEL, 0, -1);
}

void EditView::LoadRichEdit() {
  ::LoadLibraryW(L"msftedit.dll");
}

}  // namespace nu
