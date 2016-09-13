// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/view.h"

#include <tuple>
#include <utility>

#include "nativeui/container.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

namespace nu {

namespace {

// We use int to represent enums.
using IntSetter = void(*)(const CSSNodeRef, int);
using FloatSetter = void(*)(const CSSNodeRef, float);
using EdgeSetter = void(*)(const CSSNodeRef, const CSSEdge, float);

// Sorted list of CSS node properties.
std::pair<const char*, IntSetter> int_setters[] = {
  std::make_pair("aligncontent",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetAlignContent)),
  std::make_pair("alignitems",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetAlignItems)),
  std::make_pair("alignself",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetAlignSelf)),
  std::make_pair("direction",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetDirection)),
  std::make_pair("flexdirection",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetFlexDirection)),
  std::make_pair("flexwrap",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetFlexWrap)),
  std::make_pair("justifycontent",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetJustifyContent)),
  std::make_pair("overflow",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetOverflow)),
  std::make_pair("position",
                 reinterpret_cast<IntSetter>(CSSNodeStyleSetPositionType)),
};
std::pair<const char*, FloatSetter> float_setters[] = {
  std::make_pair("flexbasis",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetFlexBasis)),
  std::make_pair("flexgrow",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetFlexGrow)),
  std::make_pair("flexshrink",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetFlexShrink)),
  std::make_pair("height",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetHeight)),
  std::make_pair("maxheight",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMaxHeight)),
  std::make_pair("maxwidth",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMaxWidth)),
  std::make_pair("minheight",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMinHeight)),
  std::make_pair("minwidth",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMinWidth)),
  std::make_pair("width",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetWidth)),
};
std::tuple<const char*, CSSEdge, EdgeSetter> edge_setters[] = {
  std::make_tuple("borderbottomwidth", CSSEdgeBottom,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetBorder)),
  std::make_tuple("borderleftwidth", CSSEdgeLeft,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetBorder)),
  std::make_tuple("borderrightwidth", CSSEdgeRight,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetBorder)),
  std::make_tuple("bordertopwidth", CSSEdgeTop,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetBorder)),
  std::make_tuple("bottom", CSSEdgeBottom,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPosition)),
  std::make_tuple("left", CSSEdgeLeft,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPosition)),
  std::make_tuple("marginbottom", CSSEdgeBottom,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetMargin)),
  std::make_tuple("marginleft", CSSEdgeLeft,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetMargin)),
  std::make_tuple("marginright", CSSEdgeRight,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetMargin)),
  std::make_tuple("margintop", CSSEdgeTop,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetMargin)),
  std::make_tuple("paddingbottom", CSSEdgeBottom,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPadding)),
  std::make_tuple("paddingleft", CSSEdgeLeft,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPadding)),
  std::make_tuple("paddingright", CSSEdgeRight,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPadding)),
  std::make_tuple("paddingtop", CSSEdgeTop,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPadding)),
  std::make_tuple("right", CSSEdgeRight,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPosition)),
  std::make_tuple("top", CSSEdgeTop,
                  reinterpret_cast<EdgeSetter>(CSSNodeStyleSetPosition)),
};

// Compare function to compare between elements and keys.
template<typename T>
bool FirstCompare(const T& element, const std::string& key) {
  return std::get<0>(element) < key;
}

// Compare function to compare elements.
template<typename T>
bool ElementCompare(const T& e1, const T& e2) {
  return std::string(std::get<0>(e1)) < std::get<0>(e2);
}

// Check if the array is sorted.
template<typename T, size_t n>
bool IsSorted(T (&setters)[n]) {
  return std::is_sorted(std::begin(setters), std::end(setters),
                        ElementCompare<T>);
}

}  // namespace

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

bool View::DoSetPreferredSize(const Size& size) {
  if (preferred_size_ == size)
    return true;

  preferred_size_ = size;

  if (parent_)
    return parent_->UpdatePreferredSize();
  else
    return true;
}

void View::SetStyle(const std::string& name, const std::string& value) {
  DCHECK(IsSorted(int_setters) &&
         IsSorted(float_setters) &&
         IsSorted(edge_setters)) << "Property setters must be sorted";
}

void View::SetStyle(const std::string& name, float value) {
}

}  // namespace nu
