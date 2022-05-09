// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_RESPONDER_H_
#define NATIVEUI_RESPONDER_H_

#include "base/memory/ref_counted.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

struct MouseEvent;
struct KeyEvent;

class NATIVEUI_EXPORT Responder : public SignalDelegate,
                                  public base::RefCounted<Responder> {
 public:
  // Return the receiving responder's class name. A class name is a string
  // which uniquely identifies the class. It is intended to be used as a way to
  // find out during run time if a responder can be safely casted to a specific
  // subclass.
  virtual const char* GetClassName() const = 0;

  // Get the native object.
  NativeResponder GetNative() const { return responder_; }

  // Events.
  Signal<bool(Responder*, const MouseEvent&)> on_mouse_down;
  Signal<bool(Responder*, const MouseEvent&)> on_mouse_up;
  Signal<void(Responder*, const MouseEvent&)> on_mouse_move;
  Signal<void(Responder*, const MouseEvent&)> on_mouse_enter;
  Signal<void(Responder*, const MouseEvent&)> on_mouse_leave;
  Signal<bool(Responder*, const KeyEvent&)> on_key_down;
  Signal<bool(Responder*, const KeyEvent&)> on_key_up;

 protected:
  friend class base::RefCounted<Responder>;

  Responder();
  ~Responder() override;

  // SignalDelegate:
  void OnConnect(int identifier) override;

  // FIXME(zcbenz): Needs a more decent way to handle this.
  NativeResponder responder_;

 private:
  // Event types.
  enum { kOnMouseClick, kOnMouseMove, kOnKey };

  virtual void PlatformInstallMouseClickEvents();
  virtual void PlatformInstallMouseMoveEvents();
  void PlatformInstallKeyEvents();

  // Whether events have been installed.
  bool on_mouse_click_installed_ = false;
  bool on_mouse_move_installed_ = false;
  bool on_key_installed_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_RESPONDER_H_
