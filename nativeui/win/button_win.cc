// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include <windowsx.h>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

const int kButtonPadding = 6;

class ButtonView : public SubwinView {
 public:
  ButtonView(DWORD style, Button* delegate)
      : SubwinView(L"button", style | WS_CHILD | WS_VISIBLE | WS_TABSTOP),
        delegate_(delegate) {
  }

  void OnCommand(UINT code, int command) override {
    delegate_->on_click.Emit();
  }

 private:
  Button* delegate_;
};

}  // namespace

Button::Button(const std::string& title, Type type) {
  DWORD style = BS_PUSHBUTTON;
  if (type == CheckBox)
    style = BS_AUTOCHECKBOX;
  else if (type == Radio)
    style = BS_AUTORADIOBUTTON;
  TakeOverView(new ButtonView(style, this));
  SetTitle(title);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  SubwinView* button = static_cast<SubwinView*>(view());
  base::string16 wtitle = base::UTF8ToUTF16(title);
  SetWindowTextW(button->hwnd(), wtitle.c_str());

  // Windows doesn't have preferred size for buttons, so just add some padding.
  Size text_size = ToCeiledSize(MeasureText(view(), Font(), wtitle));
  text_size.Enlarge(DIPToPixel(kButtonPadding), DIPToPixel(kButtonPadding));
  SetPixelPreferredSize(text_size);
}

std::string Button::GetTitle() const {
  return base::UTF16ToUTF8(
      GetWindowString(static_cast<SubwinView*>(view())->hwnd()));
}

void Button::SetChecked(bool checked) {
  auto* button = static_cast<SubwinView*>(view());
  Button_SetCheck(button->hwnd(), checked ? BST_CHECKED : BST_UNCHECKED);
}

bool Button::IsChecked() const {
  auto* button = static_cast<SubwinView*>(view());
  return Button_GetCheck(button->hwnd()) == BST_CHECKED;
}

}  // namespace nu
