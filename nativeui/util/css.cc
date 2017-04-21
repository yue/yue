// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/util/css.h"

#include <algorithm>
#include <tuple>
#include <utility>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "third_party/yoga/yoga/Yoga.h"

namespace nu {

namespace {

// Converters to convert string to integer.
using IntConverter = bool(*)(const std::string&, int*);

bool AlignValue(const std::string& value, int* out) {
  if (value == "auto")
    *out = static_cast<int>(YGAlignAuto);
  else if (value == "center")
    *out = static_cast<int>(YGAlignCenter);
  else if (value == "stretch")
    *out = static_cast<int>(YGAlignStretch);
  else if (value == "flex-start")
    *out = static_cast<int>(YGAlignFlexStart);
  else if (value == "flex-end")
    *out = static_cast<int>(YGAlignFlexEnd);
  else
    return false;
  return true;
}

bool DirectionValue(const std::string& value, int* out) {
  if (value == "ltr")
    *out = static_cast<int>(YGDirectionLTR);
  else if (value == "rtl")
    *out = static_cast<int>(YGDirectionRTL);
  else if (value == "inherit")
    *out = static_cast<int>(YGDirectionInherit);
  else
    return false;
  return true;
}

bool FlexDirectionValue(const std::string& value, int* out) {
  if (value == "row")
    *out = static_cast<int>(YGFlexDirectionRow);
  else if (value == "row-reverse")
    *out = static_cast<int>(YGFlexDirectionRowReverse);
  else if (value == "column")
    *out = static_cast<int>(YGFlexDirectionColumn);
  else if (value == "column-reverse")
    *out = static_cast<int>(YGFlexDirectionColumnReverse);
  else
    return false;
  return true;
}

bool JustifyValue(const std::string& value, int* out) {
  if (value == "center")
    *out = static_cast<int>(YGJustifyCenter);
  else if (value == "space-around")
    *out = static_cast<int>(YGJustifySpaceAround);
  else if (value == "space-between")
    *out = static_cast<int>(YGJustifySpaceBetween);
  else if (value == "flex-start")
    *out = static_cast<int>(YGJustifyFlexStart);
  else if (value == "flex-end")
    *out = static_cast<int>(YGJustifyFlexEnd);
  else
    return false;
  return true;
}

bool OverflowValue(const std::string& value, int* out) {
  if (value == "visible")
    *out = static_cast<int>(YGOverflowVisible);
  else if (value == "hidden")
    *out = static_cast<int>(YGOverflowHidden);
  else
    return false;
  return true;
}

bool PositionValue(const std::string& value, int* out) {
  if (value == "absolute")
    *out = static_cast<int>(YGPositionTypeAbsolute);
  else if (value == "relative")
    *out = static_cast<int>(YGPositionTypeRelative);
  else
    return false;
  return true;
}

bool WrapValue(const std::string& value, int* out) {
  if (value == "wrap")
    *out = static_cast<int>(YGWrapWrap);
  else if (value == "nowrap")
    *out = static_cast<int>(YGWrapNoWrap);
  else
    return false;
  return true;
}

// Convert the value to pixel value.
float PixelValue(std::string value) {
  if (base::EndsWith(value, "px", base::CompareCase::SENSITIVE))
    value = value.substr(0, value.length() - 2);
  double out;
  if (!base::StringToDouble(value, &out)) {
    LOG(WARNING) << "Invalid pixel value: " << value;
    return 0;
  }
  return out;
}

// Convert the value to percent value.
int PercentValue(const std::string& value) {
  int integer;
  if (!base::StringToInt(value.substr(0, value.length() - 1), &integer)) {
    LOG(WARNING) << "Invalid percent value: " << value;
    return 0;
  }
  return integer;
}

// Easy helpers to set edge values.
void SetMargin(const YGNodeRef node, float margin) {
  YGNodeStyleSetMargin(node, YGEdgeBottom, margin);
  YGNodeStyleSetMargin(node, YGEdgeLeft, margin);
  YGNodeStyleSetMargin(node, YGEdgeRight, margin);
  YGNodeStyleSetMargin(node, YGEdgeTop, margin);
}

void SetMarginPercent(const YGNodeRef node, float margin) {
  YGNodeStyleSetMarginPercent(node, YGEdgeBottom, margin);
  YGNodeStyleSetMarginPercent(node, YGEdgeLeft, margin);
  YGNodeStyleSetMarginPercent(node, YGEdgeRight, margin);
  YGNodeStyleSetMarginPercent(node, YGEdgeTop, margin);
}

void SetPadding(const YGNodeRef node, float padding) {
  YGNodeStyleSetPadding(node, YGEdgeBottom, padding);
  YGNodeStyleSetPadding(node, YGEdgeLeft, padding);
  YGNodeStyleSetPadding(node, YGEdgeRight, padding);
  YGNodeStyleSetPadding(node, YGEdgeTop, padding);
}

void SetPaddingPercent(const YGNodeRef node, float padding) {
  YGNodeStyleSetPaddingPercent(node, YGEdgeBottom, padding);
  YGNodeStyleSetPaddingPercent(node, YGEdgeLeft, padding);
  YGNodeStyleSetPaddingPercent(node, YGEdgeRight, padding);
  YGNodeStyleSetPaddingPercent(node, YGEdgeTop, padding);
}

void SetBorderWidth(const YGNodeRef node, float border) {
  YGNodeStyleSetBorder(node, YGEdgeBottom, border);
  YGNodeStyleSetBorder(node, YGEdgeLeft, border);
  YGNodeStyleSetBorder(node, YGEdgeRight, border);
  YGNodeStyleSetBorder(node, YGEdgeTop, border);
}

// We use int to represent enums.
using IntSetter = void(*)(const YGNodeRef, int);
using FloatSetter = void(*)(const YGNodeRef, float);
using EdgeSetter = void(*)(const YGNodeRef, const YGEdge, float);

// Sorted list of CSS node properties.
const std::tuple<const char*, IntConverter, IntSetter> int_setters[] = {
  std::make_tuple("aligncontent", AlignValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetAlignContent)),
  std::make_tuple("alignitems", AlignValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetAlignItems)),
  std::make_tuple("alignself", AlignValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetAlignSelf)),
  std::make_tuple("direction", DirectionValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetDirection)),
  std::make_tuple("flexdirection", FlexDirectionValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetFlexDirection)),
  std::make_tuple("flexwrap", WrapValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetFlexWrap)),
  std::make_tuple("justifycontent", JustifyValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetJustifyContent)),
  std::make_tuple("overflow", OverflowValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetOverflow)),
  std::make_tuple("position", PositionValue,
                  reinterpret_cast<IntSetter>(YGNodeStyleSetPositionType)),
};
const std::pair<const char*, FloatSetter> float_setters[] = {
  std::make_pair("border",
                 reinterpret_cast<FloatSetter>(SetBorderWidth)),
  std::make_pair("flex",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetFlex)),
  std::make_pair("flexbasis",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetFlexBasis)),
  std::make_pair("flexgrow",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetFlexGrow)),
  std::make_pair("flexshrink",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetFlexShrink)),
  std::make_pair("height",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetHeight)),
  std::make_pair("margin",
                 reinterpret_cast<FloatSetter>(SetMargin)),
  std::make_pair("maxheight",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMaxHeight)),
  std::make_pair("maxwidth",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMaxWidth)),
  std::make_pair("minheight",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMinHeight)),
  std::make_pair("minwidth",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMinWidth)),
  std::make_pair("padding",
                 reinterpret_cast<FloatSetter>(SetPadding)),
  std::make_pair("width",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetWidth)),
};
const std::pair<const char*, FloatSetter> percent_setters[] = {
  std::make_pair("flexbasis",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetFlexBasisPercent)),
  std::make_pair("height",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetHeightPercent)),
  std::make_pair("margin",
                 reinterpret_cast<FloatSetter>(SetMarginPercent)),
  std::make_pair("maxheight",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMaxHeightPercent)),
  std::make_pair("maxwidth",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMaxWidthPercent)),
  std::make_pair("minheight",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMinHeightPercent)),
  std::make_pair("minwidth",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetMinWidthPercent)),
  std::make_pair("padding",
                 reinterpret_cast<FloatSetter>(SetPaddingPercent)),
  std::make_pair("width",
                 reinterpret_cast<FloatSetter>(YGNodeStyleSetWidthPercent)),
};
const std::tuple<const char*, YGEdge, EdgeSetter> edge_setters[] = {
  std::make_tuple("borderbottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetBorder)),
  std::make_tuple("borderleft", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetBorder)),
  std::make_tuple("borderright", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetBorder)),
  std::make_tuple("bordertop", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetBorder)),
  std::make_tuple("bottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPosition)),
  std::make_tuple("left", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPosition)),
  std::make_tuple("marginbottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMargin)),
  std::make_tuple("marginleft", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMargin)),
  std::make_tuple("marginright", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMargin)),
  std::make_tuple("margintop", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMargin)),
  std::make_tuple("paddingbottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPadding)),
  std::make_tuple("paddingleft", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPadding)),
  std::make_tuple("paddingright", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPadding)),
  std::make_tuple("paddingtop", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPadding)),
  std::make_tuple("right", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPosition)),
  std::make_tuple("top", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPosition)),
};
const std::tuple<const char*, YGEdge, EdgeSetter> edge_percent_setters[] = {
  std::make_tuple("bottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPositionPercent)),
  std::make_tuple("left", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPositionPercent)),
  std::make_tuple("marginbottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMarginPercent)),
  std::make_tuple("marginleft", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMarginPercent)),
  std::make_tuple("marginright", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMarginPercent)),
  std::make_tuple("margintop", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetMarginPercent)),
  std::make_tuple("paddingbottom", YGEdgeBottom,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPaddingPercent)),
  std::make_tuple("paddingleft", YGEdgeLeft,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPaddingPercent)),
  std::make_tuple("paddingright", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPaddingPercent)),
  std::make_tuple("paddingtop", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPaddingPercent)),
  std::make_tuple("right", YGEdgeRight,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPositionPercent)),
  std::make_tuple("top", YGEdgeTop,
                  reinterpret_cast<EdgeSetter>(YGNodeStyleSetPositionPercent)),
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
bool SetIntStyle(YGNodeRef node,
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
bool SetFloatStyle(YGNodeRef node,
                   const std::string& name,
                   const std::string& value) {
  auto* tup = Find(float_setters, name);
  if (!tup)
    return false;
  std::get<1>(*tup)(node, PixelValue(value));
  return true;
}

// Set style for percent properties.
bool SetPercentStyle(YGNodeRef node,
                     const std::string& name,
                     const std::string& value) {
  auto* tup = Find(percent_setters, name);
  if (!tup)
    return false;
  std::get<1>(*tup)(node, PercentValue(value));
  return true;
}

// Set style for edge properties.
bool SetEdgeStyle(YGNodeRef node,
                  const std::string& name,
                  const std::string& value) {
  auto* tup = Find(edge_setters, name);
  if (!tup)
    return false;
  std::get<2>(*tup)(node, std::get<1>(*tup), PixelValue(value));
  return true;
}

// Set style for edge percent properties.
bool SetEdgePercentStyle(YGNodeRef node,
                         const std::string& name,
                         const std::string& value) {
  auto* tup = Find(edge_percent_setters, name);
  if (!tup)
    return false;
  std::get<2>(*tup)(node, std::get<1>(*tup), PercentValue(value));
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

// Check whether the value is xx%.
bool IsPercentValue(const std::string& value) {
  if (value.size() < 2 || value.size() > 4)
    return false;
  return value.back() == '%';
}

}  // namespace

void SetCSSStyle(YGNodeRef node,
                 const std::string& raw_name,
                 const std::string& value) {
  DCHECK(IsSorted(int_setters) &&
         IsSorted(float_setters) &&
         IsSorted(percent_setters) &&
         IsSorted(edge_setters) &&
         IsSorted(edge_percent_setters))<< "Property setters must be sorted";
  std::string name = ParseName(raw_name);
  if (IsPercentValue(value)) {
    SetPercentStyle(node, name, value) ||
    SetEdgePercentStyle(node, name, value);
  } else {
    SetIntStyle(node, name, value) ||
    SetFloatStyle(node, name, value) ||
    SetEdgeStyle(node, name, value);
  }
}

}  // namespace nu
