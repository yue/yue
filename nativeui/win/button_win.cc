// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/button.h"

#include <windowsx.h>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/container.h"
#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/image.h"
#include "nativeui/state.h"
#include "nativeui/system.h"
#include "nativeui/win/clickable.h"

namespace nu {

namespace {

const int kButtonPadding = 1;

class ButtonImpl : public Clickable {
 public:
  ButtonImpl(Button::Type type, Button* delegate)
      : Clickable(type == Button::Type::Normal ? ControlType::Button
                     : (type == Button::Type::Checkbox ? ControlType::Checkbox
                                                       : ControlType::Radio),
                  delegate) {
    set_focusable(true);
    OnDPIChanged();  // update component size
  }

  void SetTitle(const base::string16& title) {
    text_ = new AttributedText(
        title, {TextAlign::Center, TextAlign::Center, false /* wrap */});
    UpdateTitleBounds();
    Invalidate();
  }

  void SetImage(Image* image) {
    image_ = image;
    image_size_ = image_ ? image_->GetSize() : SizeF();
    Invalidate();
  }

  SizeF GetPreferredSize() const {
    SizeF preferred_size(title_size_);
    if (image_) {
      preferred_size.Enlarge(image_size_.width(), 0);
      preferred_size.set_height(std::max(image_size_.height(),
                                         preferred_size.height()));
    } else if (type() == ControlType::Checkbox ||
               type() == ControlType::Radio) {
      preferred_size.Enlarge(box_size_.width(), 0);
    }
    preferred_size.Enlarge(kButtonPadding * 2, kButtonPadding * 2);
    return preferred_size;
  }

  void SetChecked(bool checked) {
    if (IsChecked() == checked)
      return;

    params_.checked = checked;
    Invalidate();

    // And flip all other radio buttons' state.
    View* parent = delegate()->GetParent();
    if (checked && type() == ControlType::Radio && parent &&
        parent->GetNative()->type() == ControlType::Container) {
      auto* container = static_cast<Container*>(parent);
      for (int i = 0; i < container->ChildCount(); ++i) {
        View* child = container->ChildAt(i);
        if (child != delegate() &&
            child->GetNative()->type() == ControlType::Radio)
          static_cast<Button*>(child)->SetChecked(false);
      }
    }
  }

  bool IsChecked() const {
    return params_.checked;
  }

  // Clickable:
  void OnClick() override {
    if (type() == ControlType::Checkbox)
      SetChecked(!IsChecked());
    else if (type() == ControlType::Radio && !IsChecked())
      SetChecked(true);
    Button* button = static_cast<Button*>(delegate());
    button->on_click.Emit(button);
  }

  // ViewImpl:
  void Draw(PainterWin* painter, const Rect& dirty) override {
    SizeF size = delegate()->GetBounds().size();
    SizeF preferred_size = GetPreferredSize();

    NativeTheme::ExtraParams params;
    params.button = params_;

    // Draw the button background,
    if (type() == ControlType::Button)
      painter->DrawNativeTheme(NativeTheme::Part::Button,
                               state(), Rect(size_allocation().size()), params);

    // Draw control background as a layer on button background.
    if (!background_color().transparent())
      ViewImpl::Draw(painter, dirty);

    // Checkbox and radio are left aligned.
    PointF origin;
    if (type() == ControlType::Button) {
      origin.Offset((size.width() - preferred_size.width()) / 2,
                    (size.height() - preferred_size.height()) / 2);
    } else {
      origin.Offset(0, (size.height() - preferred_size.height()) / 2);
    }

    if (image_) {
      // Draw image.
      PointF image_origin(origin);
      image_origin.set_y((size.height() - image_size_.height()) / 2.f);
      image_origin.Offset(kButtonPadding, 0);
      painter->DrawImage(image_, RectF(image_origin, image_->GetSize()));
    } else {
      // Draw the box.
      PointF box_origin = origin;
      box_origin.Offset(0, (preferred_size.height() - box_size_.height()) / 2);
      RectF box_bounds(box_origin, box_size_);
      painter->DrawNativeTheme(
          type() == ControlType::Checkbox ? NativeTheme::Part::Checkbox
                                          : NativeTheme::Part::Radio,
          state(),
          ToNearestRect(ScaleRect(box_bounds, scale_factor())),
          params);
    }

    // The bounds of text.
    RectF text_bounds(origin, preferred_size);
    text_bounds.Inset(kButtonPadding, kButtonPadding);
    if (type() == ControlType::Button || image_)
      text_bounds.Inset(image_size_.width(), 0, 0, 0);
    else
      text_bounds.Inset(box_size_.width(), 0, 0, 0);

    // The text.
    painter->DrawAttributedText(text_.get(), text_bounds);

    // Draw focused ring.
    if (HasFocus()) {
      Rect rect;
      if (type() == ControlType::Button) {
        rect = Rect(size_allocation().size());
        rect.Inset(Insets(std::ceil(1 * scale_factor())));
      } else {
        rect = ToNearestRect(ScaleRect(text_bounds, scale_factor()));
        rect.Inset(-Insets(kButtonPadding));
      }
      painter->DrawFocusRect(rect);
    }
  }

  void OnDPIChanged() override {
    // Size of checkbox.
    NativeTheme* theme = State::GetCurrent()->GetNativeTheme();
    base::win::ScopedGetDC dc(window() ? window()->hwnd() : NULL);
    Size box_size;
    if (type() == ControlType::Checkbox)
      box_size = theme->GetThemePartSize(dc, NativeTheme::Part::Checkbox,
                                         state());
    else if (type() == ControlType::Radio)
      box_size = theme->GetThemePartSize(dc, NativeTheme::Part::Radio,
                                         state());
    box_size_ = ScaleSize(SizeF(box_size), 1.f / scale_factor());
  }

  void SetFont(Font* font) override {
    ViewImpl::SetFont(font);
    UpdateTitleBounds();
  }

  void SetState(ControlState state) override {
    ViewImpl::SetState(state);
    text_->SetColor(
        System::GetColor(is_disabled() ? System::Color::DisabledText
                                       : System::Color::Text));
  }

  AttributedText* text() const { return text_.get(); }
  NativeTheme::ButtonExtraParams* params() { return &params_; }

 private:
  void UpdateTitleBounds() {
    text_->SetFont(font());
    title_size_ = text_->GetSize();
  }

  NativeTheme::ButtonExtraParams params_ = {0};

  // The size of box for radio and checkbox.
  SizeF box_size_;
  // Size of the text.
  SizeF title_size_;
  // Size of the image.
  SizeF image_size_;

  Image* image_ = nullptr;  // managed by delegate
  scoped_refptr<AttributedText> text_;
};

}  // namespace

Button::Button(const std::string& title, Type type) {
  TakeOverView(new ButtonImpl(type, this));
  SetTitle(title);
}

Button::~Button() {
}

void Button::PlatformSetTitle(const std::string& title) {
  auto* button = static_cast<ButtonImpl*>(GetNative());
  base::string16 wtitle = base::UTF8ToUTF16(title);
  button->SetTitle(wtitle);
}

std::string Button::GetTitle() const {
  return static_cast<ButtonImpl*>(GetNative())->text()->GetText();
}

void Button::SetChecked(bool checked) {
  static_cast<ButtonImpl*>(GetNative())->SetChecked(checked);
}

bool Button::IsChecked() const {
  return static_cast<ButtonImpl*>(GetNative())->IsChecked();
}

void Button::PlatformSetImage(Image* image) {
  auto* button = static_cast<ButtonImpl*>(GetNative());
  button->SetImage(image);
}

SizeF Button::GetMinimumSize() const {
  auto* button = static_cast<ButtonImpl*>(GetNative());
  return button->GetPreferredSize();
}

}  // namespace nu
