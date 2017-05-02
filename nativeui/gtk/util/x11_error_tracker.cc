// Copyright 2017 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/util/x11_error_tracker.h"

#include "base/logging.h"

namespace nu {

namespace {

unsigned char g_x11_error_code = 0;
static X11ErrorTracker* g_handler = NULL;

int X11ErrorHandler(Display* display, XErrorEvent* error) {
  g_x11_error_code = error->error_code;
  return 0;
}

}  // namespace

X11ErrorTracker::X11ErrorTracker(Display* display) : display_(display) {
  // This is a non-exhaustive check for incorrect usage. It disallows nested
  // X11ErrorTracker instances on the same thread.
  DCHECK(g_handler == NULL);
  g_handler = this;
  XSync(display, False);
  old_handler_ = XSetErrorHandler(X11ErrorHandler);
  g_x11_error_code = 0;
}

X11ErrorTracker::~X11ErrorTracker() {
  g_handler = NULL;
  XSetErrorHandler(old_handler_);
}

bool X11ErrorTracker::FoundNewError() {
  XSync(display_, False);
  unsigned char error = g_x11_error_code;
  g_x11_error_code = 0;
  return error != 0;
}

}  // namespace nu
