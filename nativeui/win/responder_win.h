// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_RESPONDER_WIN_H_
#define NATIVEUI_WIN_RESPONDER_WIN_H_

#include "nativeui/responder.h"

namespace nu {

class ResponderImpl {
 public:
  float scale_factor() const { return scale_factor_; }
  Responder* delegate() const { return delegate_; }

 protected:
  ResponderImpl(float scale_factor, Responder* delegate);

  void EmitMouseMoveEvent(NativeEvent event);
  void EmitMouseEnterEvent(NativeEvent event);
  void EmitMouseLeaveEvent(NativeEvent event);
  bool EmitMouseClickEvent(NativeEvent event);
  bool EmitKeyEvent(NativeEvent event);

  void set_scale_factor(float s) { scale_factor_ = s; }

 private:
  float scale_factor_;
  Responder* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_RESPONDER_WIN_H_
