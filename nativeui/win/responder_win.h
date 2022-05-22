// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_RESPONDER_WIN_H_
#define NATIVEUI_WIN_RESPONDER_WIN_H_

#include "nativeui/responder.h"

namespace nu {

// Possible control types.
// We only add new types when it is necessary, otherwise a new View should
// usually just be |Container| or |View|.
enum class ControlType {
  // Window types.
  Window,
  // View types.
  View,
  Subwin,
  Scrollbar,
  // Button types.
  Button,
  Checkbox,
  Radio,
  // Container types, other types should NOT be appended after Container.
  Container,
  Scroll,
};

class ResponderImpl {
 public:
  float scale_factor() const { return scale_factor_; }
  ControlType type() const { return type_; }
  Responder* delegate() const { return delegate_; }

 protected:
  ResponderImpl(float scale_factor, ControlType type, Responder* delegate);

  void EmitMouseMoveEvent(NativeEvent event);
  void EmitMouseEnterEvent(NativeEvent event);
  void EmitMouseLeaveEvent(NativeEvent event);
  bool EmitMouseClickEvent(NativeEvent event);
  bool EmitKeyEvent(NativeEvent event);

  void set_scale_factor(float s) { scale_factor_ = s; }

 private:
  float scale_factor_;
  ControlType type_;
  Responder* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_RESPONDER_WIN_H_
