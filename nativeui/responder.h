// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_RESPONDER_H_
#define NATIVEUI_RESPONDER_H_

#include "nativeui/signal.h"

namespace nu {

struct MouseEvent;
struct KeyEvent;

template<typename T>
class NATIVEUI_EXPORT Responder : public SignalDelegate {
 public:
  // Events.
  Signal<bool(T*, const MouseEvent&)> on_mouse_down;
  Signal<bool(T*, const MouseEvent&)> on_mouse_up;
  Signal<void(T*, const MouseEvent&)> on_mouse_move;
  Signal<void(T*, const MouseEvent&)> on_mouse_enter;
  Signal<void(T*, const MouseEvent&)> on_mouse_leave;
  Signal<bool(T*, const KeyEvent&)> on_key_down;
  Signal<bool(T*, const KeyEvent&)> on_key_up;

 protected:
  // Event types.
  enum { kOnMouseClick, kOnMouseMove, kOnKey };

#if defined(OS_LINUX)
  Responder();

  // SignalDelegate:
  void OnConnect(int identifier) override;
#endif

 private:
#if defined(OS_LINUX)
  // Whether events have been installed.
  bool on_mouse_click_installed_ = false;
  bool on_mouse_move_installed_ = false;
  bool on_key_installed_ = false;
  bool on_drop_installed_ = false;
#endif
};

}  // namespace nu

#endif  // NATIVEUI_RESPONDER_H_
