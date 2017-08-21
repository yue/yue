// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/clickable.h"

#include "nativeui/events/win/event_win.h"
#include "nativeui/win/window_win.h"

namespace nu {

Clickable::Clickable(ControlType type, View* delegate)
    : ViewImpl(type, delegate) {
}

Clickable::~Clickable() {
}

void Clickable::OnMouseEnter(NativeEvent event) {
  is_hovering_ = true;
  SetState(is_capturing_ ? ControlState::Pressed : ControlState::Hovered);
  ViewImpl::OnMouseEnter(event);
}

void Clickable::OnMouseMove(NativeEvent event) {
  if (!is_capturing_)
    return ViewImpl::OnMouseMove(event);

  bool mouse_in_view = size_allocation().Contains(Point(event->l_param));
  if (is_hovering_ && !mouse_in_view)
    OnMouseLeave(event);
  else if (!is_hovering_ && mouse_in_view)
    OnMouseEnter(event);
  else
    ViewImpl::OnMouseMove(event);
}

void Clickable::OnMouseLeave(NativeEvent event) {
  is_hovering_ = false;
  SetState(is_capturing_ ? ControlState::Hovered : ControlState::Normal);
  ViewImpl::OnMouseLeave(event);
}

bool Clickable::OnMouseClick(NativeEvent event) {
  if (ViewImpl::OnMouseClick(event))
    return true;

  if (event->message == WM_LBUTTONDOWN) {
    is_capturing_ = true;
    window()->SetCapture(this);
    SetState(ControlState::Pressed);
  } else {
    if (event->message == WM_LBUTTONUP && state() == ControlState::Pressed)
      OnClick();
    window()->ReleaseCapture();
  }
  return true;
}

void Clickable::OnCaptureLost() {
  is_capturing_ = false;
  SetState(is_hovering_ ? ControlState::Hovered : ControlState::Normal);
}

}  // namespace nu
