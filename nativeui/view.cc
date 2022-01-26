// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include <utility>

#include "base/strings/string_util.h"
#include "nativeui/container.h"
#include "nativeui/cursor.h"
#include "nativeui/gfx/font.h"
#include "nativeui/state.h"
#include "nativeui/util/yoga_util.h"
#include "nativeui/window.h"
#include "third_party/yoga/Yoga.h"

// This header required DEBUG to be defined.
#if defined(DEBUG)
#include "third_party/yoga/YGNodePrint.h"
#else
#define DEBUG
#include "third_party/yoga/YGNodePrint.h"
#undef DEBUG
#endif

namespace nu {

namespace {

// Convert case to lower and remove non-ASCII characters.
std::string ParseName(const std::string& name) {
  std::string parsed;
  parsed.reserve(name.size());
  for (char c : name) {
    if (base::IsAsciiAlpha(c))
      parsed.push_back(base::ToLowerASCII(c));
  }
  return parsed;
}

}  // namespace

// static
const char View::kClassName[] = "View";

View::View() : view_(nullptr) {
  // Create node with the default yoga config.
  yoga_config_ = YGConfigNew();
  YGConfigCopy(yoga_config_, State::GetCurrent()->yoga_config());
  node_ = YGNodeNewWithConfig(yoga_config_);
  YGNodeSetContext(node_, this);
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
  if (GetParent() && GetParent()->IsContainer())
    static_cast<Container*>(GetParent())->Layout();
}

int View::DoDrag(std::vector<Clipboard::Data> data, int operations) {
  DragOptions options;
  return DoDragWithOptions(std::move(data), operations, options);
}

void View::SetCursor(scoped_refptr<Cursor> cursor) {
  if (cursor_ == cursor)
    return;
  PlatformSetCursor(cursor.get());
  cursor_ = std::move(cursor);
}

void View::SetFont(scoped_refptr<Font> font) {
  if (font_ == font)
    return;
  PlatformSetFont(font.get());
  font_ = std::move(font);
  UpdateDefaultStyle();
}

void View::UpdateDefaultStyle() {
  SizeF min_size = GetMinimumSize();
  YGNodeStyleSetMinWidth(node_, min_size.width());
  YGNodeStyleSetMinHeight(node_, min_size.height());
  Layout();
}

void View::SetStyleProperty(const std::string& name, const std::string& value) {
  std::string key(ParseName(name));
  if (key == "color")
    SetColor(Color(value));
  else if (key == "backgroundcolor")
    SetBackgroundColor(Color(value));
  else
    SetYogaProperty(node_, key, value);
}

void View::SetStyleProperty(const std::string& name, float value) {
  SetYogaProperty(node_, ParseName(name), value);
}

std::string View::GetComputedLayout() const {
  std::string result;
  auto options = static_cast<YGPrintOptions>(YGPrintOptionsLayout |
                                             YGPrintOptionsStyle |
                                             YGPrintOptionsChildren);
  facebook::yoga::YGNodeToString(result, node_, options, 0);
  return result;
}

SizeF View::GetMinimumSize() const {
  return SizeF();
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

void View::BecomeContentView() {
  window_ = nullptr;
  parent_ = nullptr;
}

bool View::IsContainer() const {
  return false;
}

void View::OnSizeChanged() {
  on_size_changed.Emit(this);
}

}  // namespace nu
