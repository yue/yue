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

void TextEdit::SetText(const std::string& text) {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SetWindowTextW(hwnd, base::UTF8ToUTF16(text).c_str());
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
  ::SetWindowTextW(hwnd, L"");
}

std::tuple<int, int> TextEdit::GetSelectionRange() const {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  int start, end;
  ::SendMessage(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&start),
                                 reinterpret_cast<LPARAM>(&end));
  if (start == end)
    return std::make_tuple(-1, -1);
  else
    return std::make_tuple(start, end);
}

void TextEdit::SelectRange(int start, int end) {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, EM_SETSEL, start, end);
  ::SendMessage(hwnd, EM_SCROLLCARET, 0, 0L);
}

std::string TextEdit::GetTextInRange(int start, int end) const {
  return GetText().substr(start, end - start);
}

void TextEdit::InsertText(const std::string& text) {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessageW(hwnd, EM_REPLACESEL, FALSE,
                 reinterpret_cast<LPARAM>(base::UTF8ToUTF16(text).c_str()));
}

void TextEdit::InsertTextAt(const std::string& text, int pos) {
  SelectRange(pos, pos);
  InsertText(text);
}

void TextEdit::Delete() {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(""));
}

void TextEdit::DeleteRange(int start, int end) {
  SelectRange(start, end);
  InsertText("");
}

}  // namespace nu
