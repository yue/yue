// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/drag_drop/drop_target.h"

#include <shlobj.h>

#include "base/logging.h"
#include "nativeui/gfx/geometry/point.h"
#include "nativeui/state.h"
#include "nativeui/win/drag_operation_win.h"

namespace nu {

IDropTargetHelper* DropTarget::cached_drop_target_helper_ = nullptr;

DropTarget::DropTarget(HWND hwnd, Delegate* delegate)
    : hwnd_(hwnd), delegate_(delegate) {
  DCHECK(hwnd);
  State::GetCurrent()->InitializeCOM();
  HRESULT result = ::RegisterDragDrop(hwnd, this);
  DCHECK(SUCCEEDED(result));
}

DropTarget::~DropTarget() {}

// static
IDropTargetHelper* DropTarget::DropHelper() {
  if (!cached_drop_target_helper_) {
    CoCreateInstance(CLSID_DragDropHelper, 0, CLSCTX_INPROC_SERVER,
                     IID_IDropTargetHelper,
                     reinterpret_cast<void**>(&cached_drop_target_helper_));
  }
  return cached_drop_target_helper_;
}

///////////////////////////////////////////////////////////////////////////////
// DropTarget, IDropTarget implementation:

HRESULT DropTarget::DragEnter(IDataObject* data_object,
                              DWORD key_state,
                              POINTL cursor_position,
                              DWORD* effect) {
  // Tell the helper that we entered so it can update the drag image.
  IDropTargetHelper* drop_helper = DropHelper();
  if (drop_helper) {
    drop_helper->DragEnter(hwnd(), data_object,
                           reinterpret_cast<POINT*>(&cursor_position), *effect);
  }

  POINT screen_pt = { cursor_position.x, cursor_position.y };
  ::ScreenToClient(hwnd(), &screen_pt);

  current_data_object_ = data_object;
  last_drag_state_ = {key_state, cursor_position};
  int r = delegate_->OnDragEnter(
      current_data_object_.get(), *effect, Point(screen_pt));
  *effect = last_drag_effect_ =
      r == DRAG_OPERATION_UNHANDLED ? DRAG_OPERATION_NONE : r;
  return S_OK;
}

HRESULT DropTarget::DragOver(DWORD key_state,
                             POINTL cursor_position,
                             DWORD* effect) {
  // Tell the helper that we moved over it so it can update the drag image.
  IDropTargetHelper* drop_helper = DropHelper();
  if (drop_helper)
    drop_helper->DragOver(reinterpret_cast<POINT*>(&cursor_position), *effect);

  // Do not repeatly emit DragOver event if input state is not changed.
  if (last_drag_effect_ >= 0 &&
      key_state == last_drag_state_.key_state &&
      cursor_position.x == last_drag_state_.cursor_position.x &&
      cursor_position.y == last_drag_state_.cursor_position.y) {
    *effect = last_drag_effect_;
    return S_OK;
  }

  POINT screen_pt = { cursor_position.x, cursor_position.y };
  ::ScreenToClient(hwnd(), &screen_pt);

  last_drag_state_ = {key_state, cursor_position};
  int r = delegate_->OnDragOver(
      current_data_object_.get(), *effect, Point(screen_pt));
  *effect = last_drag_effect_ =
      r == DRAG_OPERATION_UNHANDLED ? DRAG_OPERATION_NONE : r;
  return S_OK;
}

HRESULT DropTarget::DragLeave() {
  // Tell the helper that we moved out of it so it can update the drag image.
  IDropTargetHelper* drop_helper = DropHelper();
  if (drop_helper)
    drop_helper->DragLeave();

  delegate_->OnDragLeave(current_data_object_.get());

  current_data_object_ = nullptr;
  last_drag_effect_ = -1;
  return S_OK;
}

HRESULT DropTarget::Drop(IDataObject* data_object,
                         DWORD key_state,
                         POINTL cursor_position,
                         DWORD* effect) {
  // Tell the helper that we dropped onto it so it can update the drag image.
  IDropTargetHelper* drop_helper = DropHelper();
  if (drop_helper) {
    drop_helper->Drop(current_data_object_.get(),
                      reinterpret_cast<POINT*>(&cursor_position), *effect);
  }

  POINT screen_pt = { cursor_position.x, cursor_position.y };
  ::ScreenToClient(hwnd(), &screen_pt);

  int r = delegate_->OnDrop(
      current_data_object_.get(), *effect, Point(screen_pt));
  *effect = r == DRAG_OPERATION_UNHANDLED ? DRAG_OPERATION_NONE : r;
  last_drag_effect_ = -1;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// DropTarget, IUnknown implementation:

HRESULT DropTarget::QueryInterface(const IID& iid, void** object) {
  *object = nullptr;
  if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDropTarget)) {
    *object = this;
  } else {
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

ULONG DropTarget::AddRef() {
  return ++ref_count_;
}

ULONG DropTarget::Release() {
  if (--ref_count_ == 0) {
    delete this;
    return 0U;
  }
  return ref_count_;
}

}  // namespace nu
