// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_DRAGGING_INFO_H_
#define NATIVEUI_DRAGGING_INFO_H_

#include "base/memory/weak_ptr.h"
#include "nativeui/clipboard.h"

#if defined(OS_MACOSX)
#include "nativeui/mac/drag_operation_mac.h"
#elif defined(OS_LINUX)
#include "nativeui/gtk/drag_operation_gtk.h"
#elif defined(OS_WIN)
#include "nativeui/win/drag_operation_win.h"
#endif

namespace nu {

class NATIVEUI_EXPORT DraggingInfo {
 public:
  using Data = Clipboard::Data;

  virtual ~DraggingInfo();

  virtual bool IsDataAvailable(Data::Type type) const = 0;
  virtual Data GetData(Data::Type type) const = 0;

  int GetDragOperation() const { return drag_operation_; }

  base::WeakPtr<DraggingInfo> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 protected:
  explicit DraggingInfo(int drag_operation);

 private:
  int drag_operation_ = DRAG_OPERATION_NONE;

  base::WeakPtrFactory<DraggingInfo> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DraggingInfo);
};

}  // namespace nu

#endif  // NATIVEUI_DRAGGING_INFO_H_
