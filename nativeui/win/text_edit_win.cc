// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

class TextEditImpl : public SubwinView {
 public:
  explicit TextEditImpl(TextEdit* delegate)
      : SubwinView(delegate,
                   L"edit",
                   WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE |
                   WS_CHILD | WS_VISIBLE,
                   WS_EX_CLIENTEDGE) {
    set_focusable(true);
  }

  void OnCommand(UINT code, int command) override {
    TextEdit* edit = static_cast<TextEdit*>(delegate());
    if (code == EN_CHANGE)
      edit->on_text_change.Emit(edit);
  }
};

}  // namespace

TextEdit::TextEdit() {
  TakeOverView(new TextEditImpl(this));
}

TextEdit::~TextEdit() {
}

std::string TextEdit::GetText() const {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  return base::UTF16ToUTF8(GetWindowString(hwnd));
}

void TextEdit::Cut() {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, WM_CUT, 0, 0L);
}

void TextEdit::Copy() {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, WM_COPY, 0, 0L);
}

void TextEdit::Paste() {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, WM_PASTE, 0, 0L);
}

void TextEdit::Clear() {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, WM_CLEAR, 0, 0L);
}

}  // namespace nu
