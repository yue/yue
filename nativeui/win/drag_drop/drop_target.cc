// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/drag_drop/drop_target.h"

#include <shlobj.h>

#include "base/logging.h"
#include "nativeui/state.h"

namespace nu {

IDropTargetHelper* DropTarget::cached_drop_target_helper_ = nullptr;

DropTarget::DropTarget() : hwnd_(nullptr), ref_count_(0) {}

DropTarget::~DropTarget() {}

void DropTarget::Init(HWND hwnd) {
  DCHECK(!hwnd_);
  DCHECK(hwnd);
  State::GetCurrent()->InitializeCOM();
  HRESULT result = ::RegisterDragDrop(hwnd, this);
  DCHECK(SUCCEEDED(result));
}

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
    drop_helper->DragEnter(GetHWND(), data_object,
                           reinterpret_cast<POINT*>(&cursor_position), *effect);
  }

  current_data_object_ = data_object;
  POINT screen_pt = { cursor_position.x, cursor_position.y };
  *effect =
      OnDragEnter(current_data_object_.get(), key_state, screen_pt, *effect);
  return S_OK;
}

HRESULT DropTarget::DragOver(DWORD key_state,
                             POINTL cursor_position,
                             DWORD* effect) {
  // Tell the helper that we moved over it so it can update the drag image.
  IDropTargetHelper* drop_helper = DropHelper();
  if (drop_helper)
    drop_helper->DragOver(reinterpret_cast<POINT*>(&cursor_position), *effect);

  POINT screen_pt = { cursor_position.x, cursor_position.y };
  *effect =
      OnDragOver(current_data_object_.get(), key_state, screen_pt, *effect);
  return S_OK;
}

HRESULT DropTarget::DragLeave() {
  // Tell the helper that we moved out of it so it can update the drag image.
  IDropTargetHelper* drop_helper = DropHelper();
  if (drop_helper)
    drop_helper->DragLeave();

  OnDragLeave(current_data_object_.get());

  current_data_object_ = nullptr;
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
  *effect = OnDrop(current_data_object_.get(), key_state, screen_pt, *effect);
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

DWORD DropTarget::OnDragEnter(IDataObject* data_object,
                              DWORD key_state,
                              POINT cursor_position,
                              DWORD effect) {
  LOG(ERROR) << "OnDragEnter";
  return DROPEFFECT_NONE;
}

DWORD DropTarget::OnDragOver(IDataObject* data_object,
                             DWORD key_state,
                             POINT cursor_position,
                             DWORD effect) {
  LOG(ERROR) << "OnDragOver";
  return DROPEFFECT_NONE;
}

void DropTarget::OnDragLeave(IDataObject* data_object) {
  LOG(ERROR) << "OnDragLeave";
}

DWORD DropTarget::OnDrop(IDataObject* data_object,
                         DWORD key_state,
                         POINT cursor_position,
                         DWORD effect) {
  LOG(ERROR) << "OnDrop";
  return DROPEFFECT_NONE;
}

}  // namespace nu
