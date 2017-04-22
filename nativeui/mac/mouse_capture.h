// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_MOUSE_CAPTURE_H_
#define NATIVEUI_MAC_MOUSE_CAPTURE_H_

#import <Cocoa/Cocoa.h>

#include "base/macros.h"

namespace nu {

class View;

// Simulating the SetCapture of Windows.
class MouseCapture {
 public:
  explicit MouseCapture(View* view);
  ~MouseCapture();

 private:
  void ProcessCapturedMouseEvent(NSEvent* event);

  View* view_;
  id local_monitor_;
  id global_monitor_;

  DISALLOW_COPY_AND_ASSIGN(MouseCapture);
};

}  // namespace nu

#endif  // NATIVEUI_MAC_MOUSE_CAPTURE_H_
