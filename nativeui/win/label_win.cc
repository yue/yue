// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "nativeui/win/view_win.h"

namespace nu {

namespace {

class LabelImpl : public ViewImpl {
 public:
  explicit LabelImpl(Label* delegate)
      : ViewImpl(ControlType::View, delegate) {}

  // ViewImpl:
  void Draw(PainterWin* painter, const Rect& dirty) override {
    ViewImpl::Draw(painter, dirty);

    auto* label = static_cast<Label*>(delegate());
    painter->DrawAttributedText(
        label->GetAttributedText(), RectF(label->GetBounds().size()));
  }
};

}  // namespace

NativeView Label::PlatformCreate() {
  return new LabelImpl(this);
}

void Label::PlatformSetAttributedText(AttributedText* text) {
}

}  // namespace nu
