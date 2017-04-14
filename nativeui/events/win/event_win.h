// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_EVENTS_WIN_EVENT_WIN_H_
#define NATIVEUI_EVENTS_WIN_EVENT_WIN_H_

#include <windows.h>

namespace nu {

// Windows does not have a native Event type, so we provide our own by wrapping
// all MSG members in a struct.
struct Win32Message {
  UINT message;
  WPARAM w_param;
  LPARAM l_param;
};

}  // namespace nu

#endif  // NATIVEUI_EVENTS_WIN_EVENT_WIN_H_
