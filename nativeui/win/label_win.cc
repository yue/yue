// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/window_impl.h"

namespace nu {

Label::Label(const std::string& text) {
  set_view(new WindowImpl);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  base::string16 title = base::UTF8ToUTF16(text);
  SetWindowTextW(view()->hwnd(), title.c_str());
}

std::string Label::GetText() {
  base::string16 title;
  int length = GetWindowTextLength(view()->hwnd()) + 1;
  GetWindowTextW(view()->hwnd(), base::WriteInto(&title, length), length);
  return base::UTF16ToUTF8(title);
}

}  // namespace nu


