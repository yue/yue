// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

namespace nu {

Group::Group() {
  PlatformInit();
  SetContentView(new Container);
}

Group::~Group() {
}

void Group::SetContentView(Container* container) {
  content_view_ = container;
  PlatformSetContentView(container);
}

Container* Group::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
