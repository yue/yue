// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include <windowsx.h>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/state.h"
#include "nativeui/win/base_view.h"
#include "nativeui/win/util/native_theme.h"

namespace nu {

namespace {

const int kButtonPadding = 6;

class ButtonView : public BaseView {
 public:
  explicit ButtonView(Button* delegate)
      : BaseView(true),
        native_theme_(State::current()->GetNativeTheme()),
        color_(GetThemeColor(ThemeColor::Text)),
        delegate_(delegate) {}

  void SetTitle(const base::string16& title) {
    title_ = title;
  }

  base::string16 GetTitle() const {
    return title_;
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    // Draw the button background
    HDC dc = context->GetHDC();
    NativeTheme::ButtonExtraParams params = {0};
    native_theme_->PaintPushButton(dc, NativeTheme::Normal,
                                   GetWindowPixelBounds(), params);
    context->ReleaseHDC(dc);

    // Pring the text in middle of rect.
    Size text_size = ToCeiledSize(MeasureText(this, font_, title_));
    Size ctrl_size = GetPixelBounds().size();
    Point origin((ctrl_size.width() - text_size.width()) / 2,
                      (ctrl_size.height() - text_size.height()) / 2);
    origin += GetWindowPixelOrigin().OffsetFromOrigin();

    Gdiplus::SolidBrush brush(ToGdi(color_));
    context->DrawString(title_.c_str(), static_cast<int>(title_.size()),
                        font_.GetNativeFont(), ToGdi(origin), &brush);
  }

  Font font() const { return font_; }

 private:
  NativeTheme* native_theme_;
  Color color_;
  Font font_;

  base::string16 title_;

  Button* delegate_;
};

}  // namespace

Button::Button(const std::string& title, Type type) {
  TakeOverView(new ButtonView(this));
  SetTitle(title);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  auto* button = static_cast<ButtonView*>(view());
  base::string16 wtitle = base::UTF8ToUTF16(title);
  button->SetTitle(wtitle);

  Size size = ToCeiledSize(MeasureText(button, button->font(), wtitle));
  int padding = DIPToPixel(kButtonPadding);
  size.Enlarge(padding, padding);
  if (SetPixelPreferredSize(size))
    Invalidate();
}

std::string Button::GetTitle() const {
  return base::UTF16ToUTF8(static_cast<ButtonView*>(view())->GetTitle());
}

void Button::SetChecked(bool checked) {
}

bool Button::IsChecked() const {
  return false;
}

}  // namespace nu
