// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar_button.h"

#include "nativeui/state.h"

namespace nu {

ScrollBarButton::ScrollBarButton(Type type, ScrollView* scroll)
    : BaseView(ControlType::ScrollBarButton),
      theme_(State::current()->GetNativeTheme()),
      type_(type) {
}

ScrollBarButton::~ScrollBarButton() {
}

void ScrollBarButton::Draw(PainterWin* painter, const Rect& dirty) {
  HDC dc = painter->GetHDC();
  theme_->PaintScrollbarArrow(
      dc, static_cast<int>(type_), state(),
      Rect(size_allocation().size()) + painter->origin(), params_);
  painter->ReleaseHDC(dc);
}

}  // namespace nu
