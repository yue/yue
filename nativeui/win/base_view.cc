// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/base_view.h"

namespace nu {

void BaseView::SetBounds(const gfx::Rect& bounds) {
  SetPixelBounds(ScaleToEnclosingRect(bounds, scale_factor()));
}

gfx::Rect BaseView::GetBounds() {
  return ScaleToEnclosingRect(GetPixelBounds(), 1.0f / scale_factor());
}

void BaseView::Init(HWND hwnd, float scale_factor) {
  hwnd_ = hwnd;
  scale_factor_ = scale_factor;
}

}  // namespace nu
