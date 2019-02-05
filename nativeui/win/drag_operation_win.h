// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_DRAG_OPERATION_WIN_H_
#define NATIVEUI_WIN_DRAG_OPERATION_WIN_H_

namespace nu {

enum DragOperation {
  DRAG_OPERATION_NONE = 0,  // DROPEFFECT_NONE
  DRAG_OPERATION_COPY = 1,  // DROPEFFECT_COPY
  DRAG_OPERATION_MOVE = 2,  // DROPEFFECT_MOVE
  DRAG_OPERATION_LINK = 4,  // DROPEFFECT_LINK

  // Indicates the the event is not handled.
  DRAG_OPERATION_UNHANDLED = -1,
};

}  // namespace nu

#endif  // NATIVEUI_WIN_DRAG_OPERATION_WIN_H_
