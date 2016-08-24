// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "nativeui/graphics/color.h"
#include "nativeui/graphics/font.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class LabelView : public BaseView {
 public:
  LabelView() : BaseView(true),
                color_(GetThemeColor(ThemeColor::Text)),
                font_(GetDefaultFont()),
                gdi_font_family_(base::UTF8ToUTF16(font_.family).c_str()),
                gdi_font_(&gdi_font_family_, font_.size,
                          Gdiplus::FontStyleRegular, Gdiplus::UnitPixel) {
  }

  void SetText(const base::string16& text) {
    text_ = text;
  }

  base::string16 GetText() const {
    return text_;
  }

  void Draw(Gdiplus::Graphics* context, const gfx::Rect& dirty) override {
    if (!window())
      return;

    gfx::Point origin(GetWindowPixelOrigin());

    Gdiplus::PointF point(origin.x(), origin.y());
    Gdiplus::SolidBrush brush(
        Gdiplus::Color(color_.a(), color_.r(), color_.g(), color_.b()));
    context->DrawString(text_.c_str(), static_cast<int>(text_.size()),
                        &gdi_font_, point, &brush);
  }

 private:
  Color color_;
  Font font_;
  Gdiplus::FontFamily gdi_font_family_;
  Gdiplus::Font gdi_font_;

  base::string16 text_;
};

}  // namespace

Label::Label(const std::string& text) {
  set_view(new LabelView());
  SetText(text);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  LabelView* label = static_cast<LabelView*>(view());
  label->SetText(base::UTF8ToUTF16(text));
}

std::string Label::GetText() {
  LabelView* label = static_cast<LabelView*>(view());
  return base::UTF16ToUTF8(label->GetText());
}

}  // namespace nu
