// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include "nativeui/layout/fill_layout.h"

namespace nu {

Container::Container() : layout_manager_(new FillLayout) {
  PlatformInit();
}

Container::~Container() {
}

void Container::SetLayoutManager(LayoutManager* layout_manager) {
  layout_manager_ = layout_manager;
  Layout();
}

LayoutManager* Container::GetLayoutManager() const {
  return layout_manager_.get();
}

void Container::Layout() {
  DCHECK(layout_manager_.get());
  layout_manager_->Layout(this);
}

}  // namespace nu
