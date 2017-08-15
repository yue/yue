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
}

void EditView::SetPlainText() {
  ::SendMessage(hwnd(), EM_SETTEXTMODE, TM_PLAINTEXT, 0L);
}

void EditView::SetText(const std::string& text) {
  ::SetWindowTextW(hwnd(), base::UTF8ToUTF16(text).c_str());
}

std::string EditView::GetText() const {
  return base::UTF16ToUTF8(GetWindowString(hwnd()));
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

void EditView::Clear() {
  ::SetWindowTextW(hwnd(), L"");
}

void EditView::LoadRichEdit() {
  ::LoadLibraryW(L"msftedit.dll");
}

}  // namespace nu
