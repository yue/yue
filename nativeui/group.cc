// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "nativeui/gfx/geometry/insets.h"

namespace nu {

// static
const char Group::kClassName[] = "Group";

Group::Group(const std::string& title) {
  PlatformInit();
  SetContentView(new Container);
  SetTitle(title);
}

Group::~Group() {
}

bool Group::UpdatePreferredSize() {
  Size border = GetBorderPixelSize();
  Size preferred_size = GetContentView()->GetPixelPreferredSize();
  preferred_size.Enlarge(border.width(), border.height());
  if (SetPixelPreferredSize(preferred_size))
    content_view_->Layout();
  return false;
}

const char* Group::GetClassName() const {
  return kClassName;
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
