// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "nativeui/container.h"
#include "nativeui/gfx/geometry/insets.h"

namespace nu {

// static
const char Group::kClassName[] = "Group";

Group::Group(const std::string& title) {
  PlatformInit();
  SetDefaultStyle(GetBorderSize());
  SetContentView(new Container);
  SetTitle(title);
}

Group::~Group() {
}

const char* Group::GetClassName() const {
  return kClassName;
}

void Group::SetContentView(View* view) {
  if (content_view_)
    content_view_->SetParent(nullptr);
  content_view_ = view;
  content_view_->SetParent(this);
  PlatformSetContentView(view);
}

View* Group::GetContentView() const {
  return content_view_.get();
}

}  // namespace nu
