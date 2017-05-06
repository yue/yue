// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include "nativeui/container.h"
#include "nativeui/state.h"
#include "nativeui/util/yoga_util.h"
#include "nativeui/window.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

// static
const char View::kClassName[] = "View";

View::View() : view_(nullptr) {
  // Create node with the default yoga config.
  yoga_config_ = YGConfigNew();
  YGConfigCopy(yoga_config_, State::GetCurrent()->yoga_config());
  node_ = YGNodeNewWithConfig(yoga_config_);
}

View::~View() {
  PlatformDestroy();

  // Free yoga config and node.
  YGNodeFree(node_);
  YGConfigFree(yoga_config_);
}

const char* View::GetClassName() const {
  return kClassName;
}

void View::SetVisible(bool visible) {
  if (visible == IsVisible())
    return;
  PlatformSetVisible(visible);
  YGNodeStyleSetDisplay(node_, visible ? YGDisplayFlex : YGDisplayNone);
  Layout();
}

void View::Layout() {
  // By default just make parent do layout.
  if (GetParent())
    static_cast<Container*>(GetParent())->Layout();
}

void View::SetBackgroundColor(Color color) {
  background_color_ = color;
  PlatformSetBackgroundColor(color);
}

Color View::GetBackgroundColor() const {
  return background_color_;
}

void View::SetDefaultStyle(const SizeF& minimum) {
  YGNodeStyleSetMinWidth(node_, minimum.width());
  YGNodeStyleSetMinHeight(node_, minimum.height());
  Layout();
}

void View::SetStyle(const std::string& name, const std::string& value) {
  SetYogaProperty(node_, name, value);
}

void View::SetStyle(const std::string& name, float value) {
  SetYogaProperty(node_, name, value);
}

void View::PrintStyle() const {
  YGNodePrint(node_, static_cast<YGPrintOptions>(YGPrintOptionsLayout |
                                                 YGPrintOptionsStyle |
                                                 YGPrintOptionsChildren));
}

void View::SetParent(View* parent) {
  if (parent) {
    window_ = parent->window_;
    YGConfigCopy(yoga_config_, parent->yoga_config_);
  } else {
    window_ = nullptr;
  }
  parent_ = parent;
}

void View::BecomeContentView(Window* window) {
  if (window) {
    window_ = window;
    YGConfigCopy(yoga_config_, window->GetYogaConfig());
  } else {
    window_ = nullptr;
  }
  parent_ = nullptr;
}

void View::OnSizeChanged() {
  on_size_changed.Emit(this);
}

}  // namespace nu
