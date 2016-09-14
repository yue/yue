// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"
#include "nativeui/css.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

namespace nu {

// static
const char View::kClassName[] = "View";

View::View() : view_(nullptr), node_(CSSNodeNew()) {
}

View::~View() {
  PlatformDestroy();
  CSSNodeFree(node_);
}

void View::SetVisible(bool visible) {
  PlatformSetVisible(visible);
  if (!visible)
    preferred_size_ = Size();
  if (parent())
    parent()->UpdatePreferredSize();
}

bool View::UpdatePreferredSize() {
  return false;
}

const char* View::GetClassName() const {
  return kClassName;
}

void View::SetStyle(const std::string& name, const std::string& value) {
  SetCSSStyle(node_, name, value);
}

void View::PrintStyle() const {
  CSSNodePrint(node_, static_cast<CSSPrintOptions>(CSSPrintOptionsLayout |
                                                   CSSPrintOptionsStyle |
                                                   CSSPrintOptionsChildren));
}

bool View::DoSetPreferredSize(const Size& size) {
  if (preferred_size_ == size)
    return true;

  preferred_size_ = size;

  if (parent_)
    return parent_->UpdatePreferredSize();
  else
    return true;
}

}  // namespace nu
