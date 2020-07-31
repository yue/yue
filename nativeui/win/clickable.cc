// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/clickable.h"

#include "nativeui/events/event.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/win/window_win.h"

namespace nu {

Clickable::Clickable(ControlType type, View* delegate)
    : ViewImpl(type, delegate) {
}

Clickable::~Clickable() {
}

void Clickable::SetFocus(bool focus) {
  ViewImpl::SetFocus(focus);
  if (!focus && is_space_pressing_) {
    is_space_pressing_ = false;
    ResetState();
  }
}

void Clickable::OnMouseEnter(NativeEvent event) {
  is_hovering_ = true;
  if (is_enabled() && !is_space_pressing_)
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
  if (is_enabled() && !is_space_pressing_)
    ResetState();
  ViewImpl::OnMouseLeave(event);
}

bool Clickable::OnMouseClick(NativeEvent event) {
  if (ViewImpl::OnMouseClick(event))
    return true;

  if (event->message == WM_LBUTTONDOWN) {
    is_capturing_ = true;
    if (window())
      window()->SetCapture(this);
    SetState(ControlState::Pressed);
  } else {
    if (event->message == WM_LBUTTONUP && state() == ControlState::Pressed)
      OnClick();
    if (window())
      window()->ReleaseCapture();
  }
  return true;
}

void Clickable::OnCaptureLost() {
  is_capturing_ = false;
  if (!is_space_pressing_)
    ResetState();
}

bool Clickable::OnKeyEvent(NativeEvent event) {
  if (ViewImpl::OnKeyEvent(event))
    return true;
  // Only accept keyboard operation when focus ring is showing, or when current
  // view is default button.
  if ((!window() || !window()->focus_manager()->show_focus_ring()) &&
      !is_default())
    return false;
  KeyEvent client_event(event, this);
  // Pressing ENTER would trigger click immediately.
  if (client_event.type == EventType::KeyDown &&
      client_event.key == VKEY_RETURN) {
    OnClick();
    return true;
  }
  // Pressing SPACE is similar to mouse clicking.
  if (client_event.key == VKEY_SPACE) {
    if (client_event.type == EventType::KeyDown) {
      is_space_pressing_ = true;
      SetState(ControlState::Pressed);
    } else {
      is_space_pressing_ = false;
      OnClick();
      ResetState();
    }
    return true;
  }
  return false;
}

void Clickable::ResetState() {
  if (is_enabled())
    SetState(is_hovering_ ? ControlState::Hovered : ControlState::Normal);
}

}  // namespace nu
