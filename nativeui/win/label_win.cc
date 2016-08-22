// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

Label::Label(const std::string& text) {
  set_view(new SubwinView(
      base::StringPiece16(L"static"),
      WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE));
  SetText(text);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  HWND hwnd = static_cast<SubwinView*>(view())->hwnd();
  SetWindowTextW(hwnd, base::UTF8ToUTF16(text).c_str());
}

std::string Label::GetText() {
  HWND hwnd = static_cast<SubwinView*>(view())->hwnd();
  base::string16 title;
  int length = GetWindowTextLengthW(hwnd) + 1;
  GetWindowTextW(hwnd, base::WriteInto(&title, length), length);
  return base::UTF16ToUTF8(title);
}

}  // namespace nu
