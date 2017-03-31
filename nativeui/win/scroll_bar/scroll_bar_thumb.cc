// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar_thumb.h"

#include "nativeui/win/scroll_bar/scroll_bar.h"

namespace nu {

ScrollBarThumb::ScrollBarThumb(bool vertical, ScrollBar* scroll_bar)
    : ViewImpl(ControlType::ScrollBarThumb),
      vertical_(vertical),
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
  if (message == WM_LBUTTONDOWN) {
    is_capturing_ = true;
    pressed_point_ = point;
    last_value_ = scroll_bar_->GetValue();
    window()->SetCapture(this);
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
  NativeTheme::ExtraParams params;
  params.scrollbar_thumb = params_;
  painter->DrawNativeTheme(
      (vertical_ ? NativeTheme::Part::ScrollbarVerticalThumb
                 : NativeTheme::Part::ScrollbarHorizontalThumb),
      state(),
      Rect(size_allocation().size()),
      params);
}

}  // namespace nu
