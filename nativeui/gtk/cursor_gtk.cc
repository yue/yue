// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/cursor.h"

#include <gtk/gtk.h>

namespace nu {

Cursor::Cursor(Type type) {
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    cursor_ = nullptr;
    return;
  }

  const char* name = "default";
  switch (type) {
    case Type::Default:
      name = "default";
      break;
    case Type::Hand:
      name = "pointer";
      break;
    case Type::Crosshair:
      name = "crosshair";
      break;
    case Type::Progress:
      name = "progress";
      break;
    case Type::Text:
      name = "text";
      break;
    case Type::NotAllowed:
      name = "not-allowed";
      break;
    case Type::Help:
      name = "help";
      break;
    case Type::Move:
      name = "move";
      break;
    case Type::ResizeEW:
      name = "ew-resize";
      break;
    case Type::ResizeNS:
      name = "ns-resize";
      break;
    case Type::ResizeNESW:
      name = "nesw-resize";
      break;
    case Type::ResizeNWSE:
      name = "nwse-resize";
      break;
  }
  cursor_ = gdk_cursor_new_from_name(display, name);
}

Cursor::~Cursor() {
  if (cursor_)
    g_object_unref(cursor_);
}

}  // namespace nu
