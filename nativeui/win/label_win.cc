// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/base_view.h"

namespace nu {

namespace {

class LabelView : public BaseView {
 public:
  LabelView() : BaseView(ControlType::Label),
                color_(GetThemeColor(ThemeColor::Text)) {
  }

  void SetText(const base::string16& text) {
    text_ = text;
  }

  base::string16 GetText() const {
    return text_;
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    painter->DrawStringWithFlags(text_, font_, color_,
                                 Rect(size_allocation().size()),
                                 Painter::TextAlignCenter);
  }

  Font font() const { return font_; }

 private:
  Color color_;
  Font font_;

  base::string16 text_;
};

}  // namespace

Label::Label(const std::string& text) {
  TakeOverView(new LabelView());
  SetText(text);
}

Label::~Label() {
}

void Label::SetText(const std::string& text) {
  LabelView* label = static_cast<LabelView*>(view());
  base::string16 wtext = base::UTF8ToUTF16(text);
  label->SetText(wtext);

  SetPixelPreferredSize(ToCeiledSize(MeasureText(label, label->font(), wtext)));
  label->Invalidate();
}

std::string Label::GetText() {
  LabelView* label = static_cast<LabelView*>(view());
  return base::UTF16ToUTF8(label->GetText());
}

}  // namespace nu
