// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_DRAG_OPERATION_MAC_H_
#define NATIVEUI_MAC_DRAG_OPERATION_MAC_H_

namespace nu {

enum DragOperation {
  DRAG_OPERATION_NONE = 0,   // NSDragOperationNone
  DRAG_OPERATION_COPY = 1,   // NSDragOperationCopy
  DRAG_OPERATION_MOVE = 16,  // NSDragOperationMove
  DRAG_OPERATION_LINK = 2,   // NSDragOperationLink
};

}  // namespace nu

#endif  // NATIVEUI_MAC_DRAG_OPERATION_MAC_H_
