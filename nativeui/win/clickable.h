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
  void SetFocus(bool focus) override;
  void OnMouseEnter(NativeEvent event) override;
  void OnMouseMove(NativeEvent event) override;
  void OnMouseLeave(NativeEvent event) override;
  bool OnMouseClick(NativeEvent event) override;
  void OnCaptureLost() override;
  bool OnKeyEvent(NativeEvent event) override;

  void set_is_default(bool d) { is_default_ = d; }
  bool is_default() const { return is_default_; }

  bool is_hovering() const { return is_hovering_; }
  bool is_capturing() const { return is_capturing_; }

 private:
  void ResetState();

  // Whether this is default button.
  bool is_default_ = false;

  bool is_hovering_ = false;
  bool is_capturing_ = false;
  bool is_space_pressing_ = false;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_CLICKABLE_H_
