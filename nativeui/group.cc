// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "nativeui/gfx/geometry/insets.h"

namespace nu {

Group::Group(const std::string& title) {
  PlatformInit();
  SetContentView(new Container);
  SetTitle(title);
}

Group::~Group() {
}

bool Group::UpdatePreferredSize() {
  Size outer = GetBounds().size();
  Size inner = GetContentView()->GetBounds().size();
  Size preferred_size = GetContentView()->preferred_size();
  preferred_size.Enlarge(outer.width() - inner.width(),
                         outer.height() - inner.height());
  if (SetPreferredSize(preferred_size))
    content_view_->Layout();
  return false;
}

void Group::SetContentView(Container* container) {
  content_view_ = container;
  PlatformSetContentView(container);
}

Container* Group::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
