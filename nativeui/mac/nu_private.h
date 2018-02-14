// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_PRIVATE_H_
#define NATIVEUI_MAC_NU_PRIVATE_H_

#include <memory>

#include "base/mac/scoped_nsobject.h"

@class NSTrackingArea;

namespace nu {

class MouseCapture;
class View;

// A private class that holds nativeui specific private data.
// Object-C does not support multi-inheiritance, so it is impossible to add
// common data members for UI elements. Our workaround is to manually add
// this class as member for each view.
struct NUPrivate {
  NUPrivate();
  ~NUPrivate();

  View* shell = nullptr;
  bool focusable = true;
  bool draggable = false;
  bool is_content_view = false;
  bool wants_layer = false;  // default value for wantsLayer
  bool wants_layer_infected = false;  // infects the wantsLayer property
  base::scoped_nsobject<NSTrackingArea> tracking_area;
  std::unique_ptr<MouseCapture> mouse_capture;
};

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_PRIVATE_H_
