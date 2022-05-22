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

  // Capture mouse.
  void SetCapture();
  void ReleaseCapture();
  bool HasCapture() const;

  // Get the native object.
  NativeResponder GetNative() const { return responder_; }

  // The type of the responder, currently we only have View and Window, but it
  // is possible to have things like Popover and Panel in future.
  enum class Type {
    View,
    Window,
  };
  Type GetType() const { return type_; }

  // Events.
  Signal<bool(Responder*, const MouseEvent&)> on_mouse_down;
  Signal<bool(Responder*, const MouseEvent&)> on_mouse_up;
  Signal<void(Responder*, const MouseEvent&)> on_mouse_move;
  Signal<void(Responder*, const MouseEvent&)> on_mouse_enter;
  Signal<void(Responder*, const MouseEvent&)> on_mouse_leave;
  Signal<bool(Responder*, const KeyEvent&)> on_key_down;
  Signal<bool(Responder*, const KeyEvent&)> on_key_up;
  Signal<void(Responder*)> on_capture_lost;

 protected:
  friend class base::RefCounted<Responder>;

  Responder();
  ~Responder() override;

  void InitResponder(NativeResponder native, Type type);

  // SignalDelegate:
  void OnConnect(int identifier) override;

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

  Type type_;
  NativeResponder responder_ = nullptr;
};

}  // namespace nu

#endif  // NATIVEUI_RESPONDER_H_
