// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include "nativeui/gfx/geometry/size_conversions.h"

namespace nu {

// static
const char Scroll::kClassName[] = "Scroll";

Scroll::Scroll() {
  PlatformInit();
  SetContentView(new Container);
}

Scroll::~Scroll() {
}

void Scroll::SetContentView(Container* container) {
  content_view_ = container;
  content_view_->set_parent(this);

  if (container->GetBounds().IsEmpty())
    container->SetBounds(RectF(container->preferred_size()));

  PlatformSetContentView(container);
}

Container* Scroll::GetContentView() const {
  return content_view_.get();
}

SizeF Scroll::GetContentSize() const {
  return GetContentView()->GetBounds().size();
}

const char* Scroll::GetClassName() const {
  return kClassName;
}

}  // namespace nu
