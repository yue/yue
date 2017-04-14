// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/scrollbar_thumb.h"

#include "nativeui/events/win/event_win.h"
#include "nativeui/win/scrollbar/scrollbar.h"

namespace nu {

ScrollbarThumb::ScrollbarThumb(bool vertical, Scrollbar* scrollbar)
    : ViewImpl(ControlType::ScrollbarThumb, nullptr),
      vertical_(vertical),
      scrollbar_(scrollbar) {
}

ScrollbarThumb::~ScrollbarThumb() {
}

int ScrollbarThumb::GetSize() const {
  return vertical_ ? size_allocation().height() : size_allocation().width();
}

void ScrollbarThumb::OnMouseEnter() {
  is_hovering_ = true;
  if (!is_capturing_) {
    set_state(ControlState::Hovered);
    Invalidate();
  }
}

void ScrollbarThumb::OnMouseMove(UINT flags, const Point& point) {
  if (is_capturing_) {
    int offset = vertical_ ? (point.y() - pressed_point_.y())
                           : (point.x() - pressed_point_.x());
    scrollbar_->SetValue(last_value_ + offset);
  }
}

void ScrollbarThumb::OnMouseLeave() {
  is_hovering_ = false;
  if (!is_capturing_) {
    set_state(ControlState::Normal);
    Invalidate();
  }
}

bool ScrollbarThumb::OnMouseClick(NativeEvent event) {
  if (ViewImpl::OnMouseClick(event))
    return true;

  if (event->message == WM_LBUTTONDOWN) {
    is_capturing_ = true;
    pressed_point_ = Point(event->l_param);
    last_value_ = scrollbar_->GetValue();
    window()->SetCapture(this);
    set_state(ControlState::Pressed);
  } else {
    set_state(ControlState::Hovered);
  }

  Invalidate();
  return true;
}

void ScrollbarThumb::OnCaptureLost() {
  is_capturing_ = false;
  set_state(is_hovering_ ? ControlState::Hovered : ControlState::Normal);
  Invalidate();
}

void ScrollbarThumb::Draw(PainterWin* painter, const Rect& dirty) {
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
