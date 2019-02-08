// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/win/drag_drop/drag_source.h"

namespace nu {

Microsoft::WRL::ComPtr<DragSource> DragSource::Create(Delegate* delegate) {
  return Microsoft::WRL::Make<DragSource>(delegate);
}

DragSource::DragSource(Delegate* delegate) : delegate_(delegate) {}

HRESULT DragSource::QueryContinueDrag(BOOL escape_pressed, DWORD key_state) {
  if (cancel_drag_)
    return DRAGDROP_S_CANCEL;

  if (escape_pressed) {
    delegate_->OnDragSourceCancel();
    return DRAGDROP_S_CANCEL;
  }

  if (!(key_state & MK_LBUTTON)) {
    delegate_->OnDragSourceDrop();
    return DRAGDROP_S_DROP;
  }

  delegate_->OnDragSourceMove();
  return S_OK;
}

HRESULT DragSource::GiveFeedback(DWORD effect) {
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

}  // namespace nu
