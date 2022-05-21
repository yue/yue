// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_PRIVATE_H_
#define NATIVEUI_MAC_NU_PRIVATE_H_

#include <functional>
#include <memory>

#include "base/mac/scoped_nsobject.h"

@class DataProvider;
@class NSCursor;
@class NSTrackingArea;

namespace nu {

class MouseCapture;
class Responder;

// A private class that holds nativeui specific private data.
// Object-C does not support multi-inheiritance, so it is impossible to add
// common data members for UI elements. Our workaround is to manually add
// this class as member for each view.
struct NUPrivate {
  Responder* shell = nullptr;
  bool hovered = false;
  base::scoped_nsobject<NSTrackingArea> tracking_area;
  std::unique_ptr<MouseCapture> mouse_capture;

 protected:
  NUPrivate();
  ~NUPrivate();

 private:
  // Prevent heap allocation.
  void* operator new(size_t);
  void* operator new[](size_t);
  void operator delete(void *);
  void operator delete[](void*);
};

struct NUWindowPrivate : public NUPrivate {
  bool can_resize = true;
};

struct NUViewPrivate : public NUPrivate {
  bool focusable = true;
  bool draggable = false;
  bool is_content_view = false;
  bool wants_layer = false;  // default value for wantsLayer
  bool wants_layer_infected = false;  // infects the wantsLayer property
  base::scoped_nsobject<NSCursor> cursor;

  // Drag target properties.
  int last_drop_operation = -1;  // cached drop result

  // Drag source properties.
  int supported_drag_operation = 0;  // supported drag operation
  int drag_result = 0;  // cached drag result
  std::function<void()> quit_dragging;  // quit current drag session
  base::scoped_nsobject<DataProvider> data_source;  // data of drag source
};

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_PRIVATE_H_
