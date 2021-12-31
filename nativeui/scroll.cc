// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include <utility>

#include "nativeui/container.h"
#include "nativeui/gfx/geometry/size_conversions.h"

namespace nu {

// static
const char Scroll::kClassName[] = "Scroll";

Scroll::Scroll() {
  PlatformInit();
  SetContentView(new Container);
  on_scroll.SetDelegate(this, kOnScroll);
}

Scroll::~Scroll() {
}

void Scroll::SetContentView(scoped_refptr<View> view) {
  if (content_view_)
    content_view_->SetParent(nullptr);
  PlatformSetContentView(view.get());
  content_view_ = std::move(view);
  content_view_->SetParent(this);
}

View* Scroll::GetContentView() const {
  return content_view_.get();
}

SizeF Scroll::GetContentSize() const {
  return GetContentView()->GetBounds().size();
}

const char* Scroll::GetClassName() const {
  return kClassName;
}

void Scroll::OnConnect(int identifier) {
  View::OnConnect(identifier);
  if (identifier == kOnScroll)
    SubscribeOnScroll();
}

}  // namespace nu
