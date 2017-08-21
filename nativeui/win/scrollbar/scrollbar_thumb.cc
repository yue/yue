// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/scrollbar_thumb.h"

#include "nativeui/events/win/event_win.h"
#include "nativeui/win/scrollbar/scrollbar.h"

namespace nu {

ScrollbarThumb::ScrollbarThumb(bool vertical, Scrollbar* scrollbar)
    : Clickable(ControlType::View, nullptr),
      vertical_(vertical),
      scrollbar_(scrollbar) {
}

ScrollbarThumb::~ScrollbarThumb() {
}

int ScrollbarThumb::GetSize() const {
  return vertical_ ? size_allocation().height() : size_allocation().width();
}

void ScrollbarThumb::OnMouseMove(NativeEvent event) {
  if (is_capturing()) {
    Point point(event->l_param);
    int offset = vertical_ ? (point.y() - pressed_point_.y())
                           : (point.x() - pressed_point_.x());
    scrollbar_->SetValue(last_value_ + offset);
  }
  Clickable::OnMouseMove(event);
}

bool ScrollbarThumb::OnMouseClick(NativeEvent event) {
  if (event->message == WM_LBUTTONDOWN) {
    pressed_point_ = Point(event->l_param);
    last_value_ = scrollbar_->GetValue();
  }
  return Clickable::OnMouseClick(event);
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
