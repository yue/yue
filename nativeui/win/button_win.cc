// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_util.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class ButtonView : public SubwinView {
 public:
  explicit ButtonView(Button* delegate)
      : SubwinView(L"button", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE),
        delegate_(delegate) {
  }

  void OnCommand(UINT code, int command) override {
    delegate_->on_click.Emit();
  }

 private:
  Button* delegate_;
};

}  // namespace

Button::Button(const std::string& title) {
  set_view(new ButtonView(this));
  SetTitle(title);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  SubwinView* button = static_cast<SubwinView*>(view());
  ::SetWindowTextW(button->hwnd(), base::UTF8ToUTF16(title).c_str());
}

std::string Button::GetTitle() const {
  SubwinView* button = static_cast<SubwinView*>(view());
  base::string16 title;
  int len = ::GetWindowTextLength(button->hwnd()) + 1;
  if (len > 1)
    ::GetWindowTextW(button->hwnd(), base::WriteInto(&title, len), len);
  return base::UTF16ToUTF8(title);
}

}  // namespace nu
