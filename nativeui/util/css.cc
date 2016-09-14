// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/util/css.h"

#include <algorithm>
#include <tuple>
#include <utility>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "third_party/css-layout/CSSLayout/CSSLayout.h"

namespace nu {

namespace {

// Converters to convert string to integer.
using IntConverter = bool(*)(const std::string&, int*);

bool AlignValue(const std::string& value, int* out) {
  if (value == "auto")
    *out = static_cast<int>(CSSAlignAuto);
  else if (value == "center")
    *out = static_cast<int>(CSSAlignCenter);
  else if (value == "stretch")
    *out = static_cast<int>(CSSAlignStretch);
  else if (value == "flex-start")
    *out = static_cast<int>(CSSAlignFlexStart);
  else if (value == "flex-end")
    *out = static_cast<int>(CSSAlignFlexEnd);
  else
    return false;
  return true;
}

bool DirectionValue(const std::string& value, int* out) {
  if (value == "ltr")
    *out = static_cast<int>(CSSDirectionLTR);
  else if (value == "rtl")
    *out = static_cast<int>(CSSDirectionRTL);
  else if (value == "inherit")
    *out = static_cast<int>(CSSDirectionInherit);
  else
    return false;
  return true;
}

bool FlexDirectionValue(const std::string& value, int* out) {
  if (value == "row")
    *out = static_cast<int>(CSSFlexDirectionRow);
  else if (value == "row-reverse")
    *out = static_cast<int>(CSSFlexDirectionRowReverse);
  else if (value == "column")
    *out = static_cast<int>(CSSFlexDirectionColumn);
  else if (value == "column-reverse")
    *out = static_cast<int>(CSSFlexDirectionColumnReverse);
  else
    return false;
  return true;
}

bool JustifyValue(const std::string& value, int* out) {
  if (value == "center")
    *out = static_cast<int>(CSSJustifyCenter);
  else if (value == "space-around")
    *out = static_cast<int>(CSSJustifySpaceAround);
  else if (value == "space-between")
    *out = static_cast<int>(CSSJustifySpaceBetween);
  else if (value == "flex-start")
    *out = static_cast<int>(CSSJustifyFlexStart);
  else if (value == "flex-end")
    *out = static_cast<int>(CSSJustifyFlexEnd);
  else
    return false;
  return true;
}

bool OverflowValue(const std::string& value, int* out) {
  if (value == "visible")
    *out = static_cast<int>(CSSOverflowVisible);
  else if (value == "hidden")
    *out = static_cast<int>(CSSOverflowHidden);
  else
    return false;
  return true;
}

bool PositionValue(const std::string& value, int* out) {
  if (value == "absolute")
    *out = static_cast<int>(CSSPositionTypeAbsolute);
  else if (value == "relative")
    *out = static_cast<int>(CSSPositionTypeRelative);
  else
    return false;
  return true;
}

bool WrapValue(const std::string& value, int* out) {
  if (value == "wrap")
    *out = static_cast<int>(CSSWrapTypeWrap);
  else if (value == "nowrap")
    *out = static_cast<int>(CSSWrapTypeNoWrap);
  else
    return false;
  return true;
}

// Convert the value to pixel value.
int PixelValue(std::string value) {
  if (base::EndsWith(value, "px", base::CompareCase::SENSITIVE))
    value = value.substr(0, value.length() - 2);
  int integer;
  if (!base::StringToInt(value, &integer)) {
    LOG(WARNING) << "Invalid pixel value " << value;
    return false;
  }
  return integer;
}

// Easy helpers to set edge values.
void SetMargin(const CSSNodeRef node, float margin) {
  CSSNodeStyleSetMargin(node, CSSEdgeBottom, margin);
  CSSNodeStyleSetMargin(node, CSSEdgeLeft, margin);
  CSSNodeStyleSetMargin(node, CSSEdgeRight, margin);
  CSSNodeStyleSetMargin(node, CSSEdgeTop, margin);
}

void SetPadding(const CSSNodeRef node, float padding) {
  CSSNodeStyleSetPadding(node, CSSEdgeBottom, padding);
  CSSNodeStyleSetPadding(node, CSSEdgeLeft, padding);
  CSSNodeStyleSetPadding(node, CSSEdgeRight, padding);
  CSSNodeStyleSetPadding(node, CSSEdgeTop, padding);
}

void SetBorderWidth(const CSSNodeRef node, float border) {
  CSSNodeStyleSetBorder(node, CSSEdgeBottom, border);
  CSSNodeStyleSetBorder(node, CSSEdgeLeft, border);
  CSSNodeStyleSetBorder(node, CSSEdgeRight, border);
  CSSNodeStyleSetBorder(node, CSSEdgeTop, border);
}

// We use int to represent enums.
using IntSetter = void(*)(const CSSNodeRef, int);
using FloatSetter = void(*)(const CSSNodeRef, float);
using EdgeSetter = void(*)(const CSSNodeRef, const CSSEdge, float);

// Sorted list of CSS node properties.
const std::tuple<const char*, IntConverter, IntSetter> int_setters[] = {
  std::make_tuple("aligncontent", AlignValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetAlignContent)),
  std::make_tuple("alignitems", AlignValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetAlignItems)),
  std::make_tuple("alignself", AlignValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetAlignSelf)),
  std::make_tuple("direction", DirectionValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetDirection)),
  std::make_tuple("flexdirection", FlexDirectionValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetFlexDirection)),
  std::make_tuple("flexwrap", WrapValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetFlexWrap)),
  std::make_tuple("justifycontent", JustifyValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetJustifyContent)),
  std::make_tuple("overflow", OverflowValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetOverflow)),
  std::make_tuple("position", PositionValue,
                  reinterpret_cast<IntSetter>(CSSNodeStyleSetPositionType)),
};
const std::pair<const char*, FloatSetter> float_setters[] = {
  std::make_pair("borderwidth",
                 reinterpret_cast<FloatSetter>(SetBorderWidth)),
  std::make_pair("flexbasis",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetFlexBasis)),
  std::make_pair("flexgrow",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetFlexGrow)),
  std::make_pair("flexshrink",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetFlexShrink)),
  std::make_pair("height",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetHeight)),
  std::make_pair("margin",
                 reinterpret_cast<FloatSetter>(SetMargin)),
  std::make_pair("maxheight",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMaxHeight)),
  std::make_pair("maxwidth",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMaxWidth)),
  std::make_pair("minheight",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMinHeight)),
  std::make_pair("minwidth",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetMinWidth)),
  std::make_pair("padding",
                 reinterpret_cast<FloatSetter>(SetPadding)),
  std::make_pair("width",
                 reinterpret_cast<FloatSetter>(CSSNodeStyleSetWidth)),
};
const std::tuple<const char*, CSSEdge, EdgeSetter> edge_setters[] = {
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

// Compare function to compare between elements and keys.
template<typename T>
bool FirstCompare(const T& element, const std::string& key) {
  return std::get<0>(element) < key;
}

// Find out the setter from array.
template<typename T, size_t n>
T* Find(T (&setters)[n], const std::string& name) {
  auto iter = std::lower_bound(std::begin(setters), std::end(setters), name,
                               FirstCompare<T>);
  if (iter == std::end(setters) || name != std::get<0>(*iter))
    return nullptr;
  return &(*iter);
}

// Set style for int properties.
bool SetIntStyle(CSSNodeRef node,
                 const std::string& name,
                 const std::string& value) {
  auto* tup = Find(int_setters, name);
  if (!tup)
    return false;
  int converted;
  if (!std::get<1>(*tup)(value, &converted)) {
    LOG(WARNING) << "Invalid value " << value << " for property " << name;
    return false;
  }
  std::get<2>(*tup)(node, converted);
  return true;
}

// Set style for float properties.
bool SetFloatStyle(CSSNodeRef node,
                   const std::string& name,
                   const std::string& value) {
  auto* tup = Find(float_setters, name);
  if (!tup)
    return false;
  std::get<1>(*tup)(node, PixelValue(value));
  return true;
}

// Set style for edge properties.
bool SetEdgeStyle(CSSNodeRef node,
                  const std::string& name,
                  const std::string& value) {
  auto* tup = Find(edge_setters, name);
  if (!tup)
    return false;
  std::get<2>(*tup)(node, std::get<1>(*tup), PixelValue(value));
  return true;
}

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

void SetCSSStyle(CSSNodeRef node,
                 const std::string& raw_name,
                 const std::string& value) {
  DCHECK(IsSorted(int_setters) &&
         IsSorted(float_setters) &&
         IsSorted(edge_setters)) << "Property setters must be sorted";
  std::string name = ParseName(raw_name);
  SetIntStyle(node, name, value) ||
  SetFloatStyle(node, name, value) ||
  SetEdgeStyle(node, name, value);
}

}  // namespace nu
