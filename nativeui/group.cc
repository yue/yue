// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "nativeui/gfx/geometry/insets.h"

namespace nu {

Group::Group(const std::string& title) {
  PlatformInit();
  SetContentView(new Container);

  // Give group a initialize size otherwise we will be unable to calculate the
  // preferred size.
  SetBounds(Rect(0, 0, 100, 100));

  SetTitle(title);
}

Group::~Group() {
}

bool Group::UpdatePreferredSize() {
  Size border = GetBorderSize();
  Size preferred_size = GetContentView()->preferred_size();
  preferred_size.Enlarge(border.width(), border.height());
  if (SetPreferredSize(preferred_size))
    content_view_->Layout();
  return false;
}

void Group::SetContentView(Container* container) {
  content_view_ = container;
  content_view_->set_parent(this);
  PlatformSetContentView(container);
  UpdatePreferredSize();
}

Container* Group::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
