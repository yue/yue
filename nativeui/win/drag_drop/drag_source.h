// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_DRAG_DROP_DRAG_SOURCE_H_
#define NATIVEUI_WIN_DRAG_DROP_DRAG_SOURCE_H_

#include <objidl.h>
#include <wrl/client.h>
#include <wrl/implements.h>

#include "base/memory/ref_counted.h"

namespace nu {

// A base IDropSource implementation. Handles notifications sent by an active
// drag-drop operation as the user mouses over other drop targets on their
// system. This object tells Windows whether or not the drag should continue,
// and supplies the appropriate cursors.
class DragSource
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          IDropSource> {
 public:
  class Delegate {
   public:
    virtual void OnDragSourceCancel() = 0;
    virtual void OnDragSourceDrop() = 0;
    virtual void OnDragSourceMove() = 0;
  };

  // Factory method to avoid exporting the class and all it derives from.
  static Microsoft::WRL::ComPtr<DragSource> Create(Delegate* delegate);

  // Use Create() to construct these objects. Direct calls to the constructor
  // are an error - it is only public because a WRL helper function creates the
  // objects.
  explicit DragSource(Delegate* delegate);
  ~DragSource() override {}

  DragSource& operator=(const DragSource&) = delete;
  DragSource(const DragSource&) = delete;

  // Stop the drag operation at the next chance we get.  This doesn't
  // synchronously stop the drag (since Windows is controlling that),
  // but lets us tell Windows to cancel the drag the next chance we get.
  void CancelDrag() {
    cancel_drag_ = true;
  }

  // IDropSource implementation:
  HRESULT __stdcall QueryContinueDrag(BOOL escape_pressed,
                                      DWORD key_state) override;
  HRESULT __stdcall GiveFeedback(DWORD effect) override;

 private:
  // Set to true if we want to cancel the drag operation.
  bool cancel_drag_ = false;

  Delegate* delegate_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_DRAG_DROP_DRAG_SOURCE_H_
