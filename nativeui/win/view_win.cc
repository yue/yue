// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"
#include "nativeui/win/window_impl.h"

namespace nu {

View::View() : view_(nullptr) {
}

View::~View() {
  delete view_;
}

void View::SetBounds(const gfx::Rect& bounds) {
  view_->SetBounds(bounds);
}

gfx::Rect View::GetBounds() {
  return view_->GetBounds();
}

}  // namespace nu

