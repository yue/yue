// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/text.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class LabelView : public BaseView {
 public:
  LabelView() : BaseView(true),
                color_(GetThemeColor(ThemeColor::Text)) {
  }

  void SetText(const base::string16& text) {
    text_ = text;
  }

  base::string16 GetText() const {
    return text_;
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    if (!window())
      return;

    // Pring the text in middle of rect.
    Size text_size = ToFlooredSize(MeasureText(font_, text_));
    Size ctrl_size = GetPixelBounds().size();
    Point origin((ctrl_size.width() - text_size.width()) / 2,
                      (ctrl_size.height() - text_size.height()) / 2);
    origin += GetWindowPixelOrigin().OffsetFromOrigin();

    Gdiplus::SolidBrush brush(ToGdi(color_));
    context->DrawString(text_.c_str(), static_cast<int>(text_.size()),
                        font_.GetNativeFont(), ToGdi(origin), &brush);
  }

 private:
  Color color_;
  Font font_;

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
  label->Invalidate();
}

std::string Label::GetText() {
  LabelView* label = static_cast<LabelView*>(view());
  return base::UTF16ToUTF8(label->GetText());
}

}  // namespace nu
