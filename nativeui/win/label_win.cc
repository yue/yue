// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/view_win.h"

namespace nu {

namespace {

class LabelImpl : public ViewImpl {
 public:
  explicit LabelImpl(Label* delegate)
      : ViewImpl(ControlType::View, delegate) {}

  void SetText(const base::string16& text) {
    text_ = text;
    Invalidate();
  }

  base::string16 GetText() const {
    return text_;
  }

  void SetAlign(TextAlign align) {
    align_ = align;
    Invalidate();
  }

  void SetVAlign(TextAlign align) {
    valign_ = align;
    Invalidate();
  }

  // ViewImpl:
  void Draw(PainterWin* painter, const Rect& dirty) override {
    ViewImpl::Draw(painter, dirty);
    TextAttributes attributes(font(), color(), align_, valign_);
    painter->DrawTextPixel(text_, Rect(size_allocation().size()), attributes);
  }

 private:
  base::string16 text_;
  TextAlign align_ = TextAlign::Center;
  TextAlign valign_ = TextAlign::Center;
};

}  // namespace

Label::Label(const std::string& text) {
  TakeOverView(new LabelImpl(this));
  SetText(text);
}

Label::~Label() {
}

void Label::PlatformSetText(const std::string& text) {
  LabelImpl* label = static_cast<LabelImpl*>(GetNative());
  base::string16 wtext = base::UTF8ToUTF16(text);
  label->SetText(wtext);
}

std::string Label::GetText() const {
  LabelImpl* label = static_cast<LabelImpl*>(GetNative());
  return base::UTF16ToUTF8(label->GetText());
}

void Label::SetAlign(TextAlign align) {
  LabelImpl* label = static_cast<LabelImpl*>(GetNative());
  label->SetAlign(align);
}

void Label::SetVAlign(TextAlign align) {
  LabelImpl* label = static_cast<LabelImpl*>(GetNative());
  label->SetVAlign(align);
}

SizeF Label::GetMinimumSize() const {
  LabelImpl* label = static_cast<LabelImpl*>(GetNative());
  return ScaleSize(MeasureText(label->GetText(), label->font()),
                   1.0f / GetScaleFactor());
}

}  // namespace nu
