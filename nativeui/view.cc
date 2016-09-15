// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"
#include "nativeui/util/css.h"
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
    preferred_size_ = SizeF();
  if (parent())
    parent()->UpdatePreferredSize();
}

void View::SetPreferredSize(const SizeF& minimum, const SizeF& preferred) {
  if (minimum_size_ == minimum && preferred_size_ == preferred)
    return;

  if (minimum == preferred && !minimum.IsEmpty()) {
    // Fixed-size widget.
    CSSNodeStyleSetWidth(node_, minimum.width());
    CSSNodeStyleSetHeight(node_, minimum.height());
  } else if (minimum != preferred) {
    // Flexible-size widget.
    CSSNodeStyleSetMinWidth(node_, minimum.width());
    CSSNodeStyleSetMinHeight(node_, minimum.height());
  }

  minimum_size_ = minimum;
  preferred_size_ = preferred;

  if (parent_)
    parent_->UpdatePreferredSize();
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

}  // namespace nu
