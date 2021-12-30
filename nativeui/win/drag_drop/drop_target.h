// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_WIN_DRAG_DROP_DROP_TARGET_H_
#define NATIVEUI_WIN_DRAG_DROP_DROP_TARGET_H_

#include <objidl.h>

#include "base/memory/ref_counted.h"

// Windows interface.
struct IDropTargetHelper;

namespace nu {

class Point;

// A DropTarget implementation that takes care of the nitty gritty
// of dnd. While this class is concrete, subclasses will most likely
// want to override various OnXXX methods.
//
// Because DropTarget is ref counted you shouldn't delete it directly,
// rather wrap it in a scoped_refptr. Be sure and invoke RevokeDragDrop(m_hWnd)
// before the HWND is deleted too.
//
// This class is meant to be used in a STA and is not multithread-safe.
class DropTarget : public IDropTarget {
 public:
  class Delegate {
   public:
    // Invoked when the cursor first moves over the hwnd during a dnd session.
    // This should return a bitmask of the supported drop operations:
    // DROPEFFECT_NONE, DROPEFFECT_COPY, DROPEFFECT_LINK and/or
    // DROPEFFECT_MOVE.
    virtual int OnDragEnter(IDataObject* data,
                            int effect,
                            const Point& point) = 0;

    // Invoked when the cursor moves over the window during a dnd session.
    // This should return a bitmask of the supported drop operations:
    // DROPEFFECT_NONE, DROPEFFECT_COPY, DROPEFFECT_LINK and/or
    // DROPEFFECT_MOVE.
    virtual int OnDragOver(IDataObject* data,
                           int effect,
                           const Point& point) = 0;

    // Invoked when the cursor moves outside the bounds of the hwnd during a
    // dnd session.
    virtual void OnDragLeave(IDataObject* data) = 0;

    // Invoked when the drop ends on the window. This should return the
    // operation that was taken.
    virtual int OnDrop(IDataObject* data, int effect, const Point& point) = 0;
  };

  DropTarget(HWND hwnd, Delegate* delegate);
  virtual ~DropTarget();

  DropTarget& operator=(const DropTarget&) = delete;
  DropTarget(const DropTarget&) = delete;

  // Returns the hosting HWND.
  HWND hwnd() { return hwnd_; }

  // IDropTarget implementation:
  HRESULT __stdcall DragEnter(IDataObject* data_object,
                              DWORD key_state,
                              POINTL cursor_position,
                              DWORD* effect) override;
  HRESULT __stdcall DragOver(DWORD key_state,
                             POINTL cursor_position,
                             DWORD* effect) override;
  HRESULT __stdcall DragLeave() override;
  HRESULT __stdcall Drop(IDataObject* data_object,
                         DWORD key_state,
                         POINTL cursor_position,
                         DWORD* effect) override;

  // IUnknown implementation:
  HRESULT __stdcall QueryInterface(const IID& iid, void** object) override;
  ULONG __stdcall AddRef() override;
  ULONG __stdcall Release() override;

 private:
  // Returns the cached drop helper, creating one if necessary. The returned
  // object is not addrefed. May return NULL if the object couldn't be created.
  static IDropTargetHelper* DropHelper();

  // The data object currently being dragged over this drop target.
  scoped_refptr<IDataObject> current_data_object_;

  // The state of last DragOver.
  struct {
    DWORD key_state = 0;
    POINTL cursor_position = {0, 0};
  } last_drag_state_;

  // Cached effect var.
  int last_drag_effect_ = -1;

  // A helper object that is used to provide drag image support while the mouse
  // is dragging over the content area.
  //
  // DO NOT ACCESS DIRECTLY! Use DropHelper() instead, which will lazily create
  // this if it doesn't exist yet. This object can take tens of milliseconds to
  // create, and we don't want to block any window opening for this, especially
  // since often, DnD will never be used. Instead, we force this penalty to the
  // first time it is actually used.
  static IDropTargetHelper* cached_drop_target_helper_;

  LONG ref_count_ = 0;

  // The HWND of the source. This HWND is used to determine coordinates for
  // mouse events that are sent to the renderer notifying various drag states.
  HWND hwnd_;

  Delegate* delegate_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_DRAG_DROP_DROP_TARGET_H_
