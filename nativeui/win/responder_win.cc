// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/responder_win.h"

#include "nativeui/events/event.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/win/view_win.h"
#include "nativeui/win/window_win.h"

namespace nu {

ResponderImpl::ResponderImpl(float scale_factor, Responder* delegate)
  : scale_factor_(scale_factor),
    delegate_(delegate) {}

void ResponderImpl::EmitMouseMoveEvent(NativeEvent event) {
  if (!delegate() || delegate()->on_mouse_move.IsEmpty())
    return;
  event->w_param = 0;
  delegate()->on_mouse_move.Emit(delegate(), MouseEvent(event, this));
}

void ResponderImpl::EmitMouseEnterEvent(NativeEvent event) {
  if (!delegate() || delegate()->on_mouse_enter.IsEmpty())
    return;
  event->w_param = 1;
  delegate()->on_mouse_enter.Emit(delegate(), MouseEvent(event, this));
}

void ResponderImpl::EmitMouseLeaveEvent(NativeEvent event) {
  if (!delegate() || delegate()->on_mouse_leave.IsEmpty())
    return;
  event->w_param = 2;
  delegate()->on_mouse_leave.Emit(delegate(), MouseEvent(event, this));
}

bool ResponderImpl::EmitMouseClickEvent(NativeEvent event) {
  if (!delegate())
    return false;
  MouseEvent client_event(event, this);
  if (client_event.type == EventType::MouseDown &&
      delegate()->on_mouse_down.Emit(delegate(), client_event))
    return true;
  if (client_event.type == EventType::MouseUp &&
      delegate()->on_mouse_up.Emit(delegate(), client_event))
    return true;
  return false;
}

bool ResponderImpl::EmitKeyEvent(NativeEvent event) {
  if (!delegate())
    return false;
  KeyEvent client_event(event, this);
  if (client_event.type == EventType::KeyDown &&
      delegate()->on_key_down.Emit(delegate(), client_event))
    return true;
  if (client_event.type == EventType::KeyUp &&
      delegate()->on_key_up.Emit(delegate(), client_event))
    return true;
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Public Responder API implementation.

void Responder::SetCapture() {
  if (GetClassName() == Window::kClassName) {
    auto* win = static_cast<WindowImpl*>(GetNative());
    win->SetCapture();
  } else {
    auto* view = static_cast<ViewImpl*>(GetNative());
    if (view->window())
      view->window()->SetCapture(view);
  }
}

void Responder::ReleaseCapture() {
  if (GetClassName() == Window::kClassName) {
    auto* win = static_cast<WindowImpl*>(GetNative());
    win->ReleaseCapture();
  } else {
    auto* view = static_cast<ViewImpl*>(GetNative());
    if (view->window())
      view->window()->ReleaseCapture();
  }
}

bool Responder::HasCapture() const {
  if (GetClassName() == Window::kClassName) {
    auto* win = static_cast<WindowImpl*>(GetNative());
    return win->HasCapture();
  } else {
    auto* view = static_cast<ViewImpl*>(GetNative());
    return view->window() && view->window()->captured_view() == view;
  }
}

void Responder::PlatformInstallMouseClickEvents() {
}

void Responder::PlatformInstallMouseMoveEvents() {
}

void Responder::PlatformInstallKeyEvents() {
}

}  // namespace nu
