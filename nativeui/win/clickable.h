// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_CLICKABLE_H_
#define NATIVEUI_WIN_CLICKABLE_H_

#include "nativeui/win/view_win.h"

namespace nu {

// Implements common operations of a clickable view.
class Clickable : public ViewImpl {
 public:
  Clickable(ControlType type, View* delegate);
  ~Clickable() override;

  virtual void OnClick() {}

  // ViewImpl:
  void OnMouseEnter(NativeEvent event) override;
  void OnMouseMove(NativeEvent event) override;
  void OnMouseLeave(NativeEvent event) override;
  bool OnMouseClick(NativeEvent event) override;
  void OnCaptureLost() override;

  bool is_hovering() const { return is_hovering_; }
  bool is_capturing() const { return is_capturing_; }

 private:
  bool is_hovering_ = false;
  bool is_capturing_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_CLICKABLE_H_
