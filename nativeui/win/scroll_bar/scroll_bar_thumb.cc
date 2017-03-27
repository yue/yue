// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar_thumb.h"

#include "nativeui/gfx/geometry/vector2d_conversions.h"
#include "nativeui/state.h"
#include "nativeui/win/scroll_bar/scroll_bar.h"
#include "nativeui/win/window_win.h"

namespace nu {

ScrollBarThumb::ScrollBarThumb(bool vertical, ScrollBarView* scroll_bar)
    : BaseView(ControlType::ScrollBarThumb),
      vertical_(vertical),
      theme_(State::GetCurrent()->GetNativeTheme()),
      scroll_bar_(scroll_bar) {
}

ScrollBarThumb::~ScrollBarThumb() {
}

int ScrollBarThumb::GetSize() const {
  return vertical_ ? size_allocation().height() : size_allocation().width();
}

void ScrollBarThumb::OnMouseEnter() {
  is_hovering_ = true;
  if (!is_capturing_) {
    set_state(ControlState::Hovered);
    Invalidate();
  }
}

void ScrollBarThumb::OnMouseMove(UINT flags, const Point& point) {
  if (is_capturing_) {
    int offset = vertical_ ? (point.y() - pressed_point_.y())
                           : (point.x() - pressed_point_.x());
    scroll_bar_->SetValue(last_value_ + offset);
  }
}

void ScrollBarThumb::OnMouseLeave() {
  is_hovering_ = false;
  if (!is_capturing_) {
    set_state(ControlState::Normal);
    Invalidate();
  }
}

bool ScrollBarThumb::OnMouseClick(UINT message, UINT flags,
                                  const Point& point) {
  auto* toplevel_window = static_cast<TopLevelWindow*>(GetNative());
  if (message == WM_LBUTTONDOWN) {
    is_capturing_ = true;
    pressed_point_ = point;
    last_value_ = scroll_bar_->GetValue();
    toplevel_window->SetCapture(this);
    set_state(ControlState::Pressed);
  } else {
    set_state(ControlState::Hovered);
  }

  Invalidate();
  return true;
}

void ScrollBarThumb::OnCaptureLost() {
  is_capturing_ = false;
  set_state(is_hovering_ ? ControlState::Hovered : ControlState::Normal);
  Invalidate();
}

void ScrollBarThumb::Draw(PainterWin* painter, const Rect& dirty) {
  HDC dc = painter->GetHDC();
  theme_->PaintScrollbarThumb(
      dc, vertical_, state(),
      Rect(size_allocation().size()) + ToCeiledVector2d(painter->origin()),
      params_);
  painter->ReleaseHDC(dc);
}

}  // namespace nu
