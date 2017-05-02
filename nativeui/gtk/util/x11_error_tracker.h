// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_UTIL_X11_ERROR_TRACKER_H_
#define NATIVEUI_GTK_UTIL_X11_ERROR_TRACKER_H_

#include <X11/Xlib.h>

#include "base/macros.h"
#include "nativeui/gtk/util/x11_types.h"

namespace nu {

// X11ErrorTracker catches X11 errors in a non-fatal way. It does so by
// temporarily changing the X11 error handler. The old error handler is
// restored when the tracker is destroyed.
class X11ErrorTracker {
 public:
  explicit X11ErrorTracker(XDisplay* display);
  ~X11ErrorTracker();

  // Returns whether an X11 error happened since this function was last called
  // (or since the creation of the tracker). This is potentially expensive,
  // since this causes a sync with the X server.
  bool FoundNewError();

 private:
  XDisplay* display_;
  XErrorHandler old_handler_;

  DISALLOW_COPY_AND_ASSIGN(X11ErrorTracker);
};

}  // namespace nu

#endif  // NATIVEUI_GTK_UTIL_X11_ERROR_TRACKER_H_
