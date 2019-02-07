// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_DRAG_OPERATION_GTK_H_
#define NATIVEUI_GTK_DRAG_OPERATION_GTK_H_

namespace nu {

enum DragOperation {
  DRAG_OPERATION_NONE = 1 << 0,  // GDK_ACTION_DEFAULT
  DRAG_OPERATION_COPY = 1 << 1,  // GDK_ACTION_COPY
  DRAG_OPERATION_MOVE = 1 << 2,  // GDK_ACTION_MOVE
  DRAG_OPERATION_LINK = 1 << 3,  // GDK_ACTION_LINK
};

}  // namespace nu

#endif  // NATIVEUI_GTK_DRAG_OPERATION_GTK_H_
