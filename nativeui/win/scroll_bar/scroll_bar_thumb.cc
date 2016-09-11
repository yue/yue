// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar_thumb.h"

#include "nativeui/state.h"

namespace nu {

ScrollBarThumb::ScrollBarThumb(bool vertical, ScrollBarView* scroll_bar)
    : BaseView(ControlType::ScrollBarThumb),
      vertical_(vertical),
      theme_(State::current()->GetNativeTheme()),
      scroll_bar_(scroll_bar) {
}

ScrollBarThumb::~ScrollBarThumb() {
}

void ScrollBarThumb::OnMouseEnter() {
  set_state(ControlState::Hovered);
  Invalidate();
}

void ScrollBarThumb::OnMouseLeave() {
  set_state(ControlState::Normal);
  Invalidate();
}

bool ScrollBarThumb::OnMouseClick(UINT message, UINT flags, const Point&) {
  return true;
}

void ScrollBarThumb::Draw(PainterWin* painter, const Rect& dirty) {
  HDC dc = painter->GetHDC();
  theme_->PaintScrollbarThumb(
      dc, vertical_, state(),
      Rect(size_allocation().size()) + painter->origin(), params_);
  painter->ReleaseHDC(dc);
}

}  // namespace nu
