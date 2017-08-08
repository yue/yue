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
  return base::UTF16ToUTF8(
      GetWindowString(static_cast<SubwinView*>(GetNative())->hwnd()));
}

}  // namespace nu
