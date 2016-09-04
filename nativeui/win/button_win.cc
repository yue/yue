// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

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
  explicit ButtonView(Button* delegate)
      : SubwinView(L"button",
                   BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP),
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
  TakeOverView(new ButtonView(this));
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

}  // namespace nu
