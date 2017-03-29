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
  SetDefaultStyle(GetBorderSize());
}

Group::~Group() {
}

const char* Group::GetClassName() const {
  return kClassName;
}

void Group::Layout() {
  View::Layout();
  content_view_->Layout();
}

void Group::SetContentView(Container* container) {
  content_view_ = container;
  content_view_->SetParent(this);
  PlatformSetContentView(container);

  SetDefaultStyle(GetBorderSize() + container->GetPreferredSize());
  Layout();
}

Container* Group::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
