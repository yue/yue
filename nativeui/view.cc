// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"

namespace nu {

void View::SetVisible(bool visible) {
  PlatformSetVisible(visible);
  if (parent())
    parent()->UpdatePreferredSize();
}

bool View::UpdatePreferredSize() {
  return false;
}

bool View::DoSetPreferredSize(const Size& size) {
  if (preferred_size_ == size)
    return true;

  preferred_size_ = size;

  if (parent_)
    return parent_->UpdatePreferredSize();
  else
    return true;
}

}  // namespace nu
