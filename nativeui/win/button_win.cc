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

const int kButtonPadding = 3;
const int kCheckBoxPadding = 1;

class ButtonView : public BaseView {
 public:
  ButtonView(Button::Type type, Button* delegate)
      : BaseView(true),
        type_(type),
        theme_(State::current()->GetNativeTheme()),
        color_(GetThemeColor(ThemeColor::Text)),
        delegate_(delegate) {
    if (type == Button::CheckBox)
      box_size_ = theme_->GetThemePartSize(NativeTheme::CheckBox, state());
    else if (type == Button::Radio)
      box_size_ = theme_->GetThemePartSize(NativeTheme::Radio, state());
  }

  void SetTitle(const base::string16& title) {
    title_ = title;
  }

  base::string16 GetTitle() const {
    return title_;
  }

  Size GetPreferredSize() const {
    Size size = ToCeiledSize(MeasureText(this, font_, title_));
    int padding = delegate_->DIPToPixel(
        type_ == Button::Normal ? kButtonPadding : kCheckBoxPadding);
    size.Enlarge(box_size_.width() + padding * 2, padding * 2);
    return size;
  }

  void OnMouseEnter() override {
    set_state(ControlState::Hovered);
    Invalidate();
  }

  void OnMouseLeave() override {
    set_state(ControlState::Normal);
    Invalidate();
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    HDC dc = context->GetHDC();

    // Draw the button background
    if (type_ == Button::Normal)
      theme_->PaintPushButton(dc, state(), GetWindowPixelBounds(), params_);

    // Place the content in the middle.
    Size preferred_size = delegate_->GetPixelPreferredSize();
    Size ctrl_size = GetPixelBounds().size();
    Point origin((ctrl_size.width() - preferred_size.width()) / 2,
                 (ctrl_size.height() - preferred_size.height()) / 2);
    origin += GetWindowPixelOrigin().OffsetFromOrigin();

    // Draw the box.
    Point box_origin = origin;
    box_origin.Offset(0, (preferred_size.height() - box_size_.height()) / 2);
    if (type_ == Button::CheckBox)
      theme_->PaintCheckBox(dc, state(), Rect(box_origin, box_size_), params_);
    else if (type_ == Button::Radio)
      theme_->PaintRadio(dc, state(), Rect(box_origin, box_size_), params_);

    context->ReleaseHDC(dc);

    // The text.
    int padding = delegate_->DIPToPixel(
        type_ == Button::Normal ? kButtonPadding : kCheckBoxPadding);
    Point text_origin = origin;
    text_origin.Offset(box_size_.width() + padding, padding);
    Gdiplus::SolidBrush brush(ToGdi(color_));
    context->DrawString(title_.c_str(), static_cast<int>(title_.size()),
                        font_.GetNativeFont(), ToGdi(text_origin), &brush);
  }

  NativeTheme::ButtonExtraParams* params() { return &params_; }
  Font font() const { return font_; }

 private:
  Button::Type type_;
  NativeTheme* theme_;

  NativeTheme::ButtonExtraParams params_ = {0};

  // The size of box for radio and checkbox.
  Size box_size_;

  // Default text color and font.
  Color color_;
  Font font_;

  base::string16 title_;
  Button* delegate_;
};

}  // namespace

Button::Button(const std::string& title, Type type) {
  TakeOverView(new ButtonView(type, this));
  SetTitle(title);
}

Button::~Button() {
}

void Button::SetTitle(const std::string& title) {
  auto* button = static_cast<ButtonView*>(view());
  base::string16 wtitle = base::UTF8ToUTF16(title);
  button->SetTitle(wtitle);

  if (SetPixelPreferredSize(button->GetPreferredSize()))
    Invalidate();
}

std::string Button::GetTitle() const {
  return base::UTF16ToUTF8(static_cast<ButtonView*>(view())->GetTitle());
}

void Button::SetChecked(bool checked) {
  auto* button = static_cast<ButtonView*>(view());
  button->params()->checked = checked;
}

bool Button::IsChecked() const {
  auto* button = static_cast<ButtonView*>(view());
  return button->params()->checked;
}

}  // namespace nu
