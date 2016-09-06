// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

namespace nu {

Scroll::Scroll(const Size& size) {
  SetPreferredSize(size);
  PlatformInit(size);
  SetContentView(new Container);
}

Scroll::~Scroll() {
}

void Scroll::SetContentView(Container* container) {
  content_view_ = container;
  content_view_->set_parent(this);
  PlatformSetContentView(container);
}

Container* Scroll::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
