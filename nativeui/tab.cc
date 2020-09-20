// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/tab.h"

#include <utility>

namespace nu {

// static
const char Tab::kClassName[] = "Tab";

Tab::Tab() {
  TakeOverView(PlatformCreate());
  UpdateDefaultStyle();
}

Tab::~Tab() {
}

void Tab::AddPage(const std::string& title, scoped_refptr<View> view) {
  if (!view || view->GetParent() == this)
    return;
  view->SetParent(this);
  pages_.emplace_back(view);
  PlatformAddPage(title, view.get());
}

void Tab::RemovePage(View* view) {
  auto it = std::find_if(pages_.begin(), pages_.end(),
                         [view](auto& p) { return p.get() == view; });
  if (it == pages_.end())
    return;
  PlatformRemovePage(it - pages_.begin(), view);
  (*it)->SetParent(nullptr);
  pages_.erase(it);
}

const char* Tab::GetClassName() const {
  return kClassName;
}

}  // namespace nu
