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

const char* View::GetClassName() const {
  return kClassName;
}

void View::SetVisible(bool visible) {
  if (visible == IsVisible())
    return;
  PlatformSetVisible(visible);
  // CSSLayout doesn't support invisible node, so we just mark the invisible
  // node as absolute to skip the layout, and restore it after it becomes
  // visible.
  if (visible) {
    CSSNodeStyleSetPositionType(node_,
                                static_cast<CSSPositionType>(node_position_));
  } else {
    node_position_ = static_cast<int>(CSSNodeStyleGetPositionType(node_));
    CSSNodeStyleSetPositionType(node_, CSSPositionTypeAbsolute);
  }
  Layout();
}

void View::Layout() {
  // By default just make parent do layout.
  if (parent())
    static_cast<Container*>(parent())->Layout();
}

void View::SetDefaultStyle(const SizeF& minimum) {
  CSSNodeStyleSetMinWidth(node_, minimum.width());
  CSSNodeStyleSetMinHeight(node_, minimum.height());
  Layout();
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
