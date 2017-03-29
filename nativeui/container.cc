// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/container.h"

#include <algorithm>
#include <limits>

#include "base/logging.h"
#include "third_party/yoga/yoga/YGNodeList.h"
#include "third_party/yoga/yoga/Yoga.h"

// Copied from Yoga.c to expose internal APIs:
extern "C" {

typedef struct YGCachedMeasurement {
  float availableWidth;
  float availableHeight;
  YGMeasureMode widthMeasureMode;
  YGMeasureMode heightMeasureMode;

  float computedWidth;
  float computedHeight;
} YGCachedMeasurement;

// This value was chosen based on empiracle data. Even the most complicated
// layouts should not require more than 16 entries to fit within the cache.
#define YG_MAX_CACHED_RESULT_COUNT 16

typedef struct YGLayout {
  float position[4];
  float dimensions[2];
  float margin[6];
  float border[6];
  float padding[6];
  YGDirection direction;

  uint32_t computedFlexBasisGeneration;
  float computedFlexBasis;

  // Instead of recomputing the entire layout every single time, we
  // cache some information to break early when nothing changed
  uint32_t generationCount;
  YGDirection lastParentDirection;

  uint32_t nextCachedMeasurementsIndex;
  YGCachedMeasurement cachedMeasurements[YG_MAX_CACHED_RESULT_COUNT];
  float measuredDimensions[2];

  YGCachedMeasurement cachedLayout;
} YGLayout;

typedef struct YGStyle {
  YGDirection direction;
  YGFlexDirection flexDirection;
  YGJustify justifyContent;
  YGAlign alignContent;
  YGAlign alignItems;
  YGAlign alignSelf;
  YGPositionType positionType;
  YGWrap flexWrap;
  YGOverflow overflow;
  YGDisplay display;
  float flex;
  float flexGrow;
  float flexShrink;
  YGValue flexBasis;
  YGValue margin[YGEdgeCount];
  YGValue position[YGEdgeCount];
  YGValue padding[YGEdgeCount];
  YGValue border[YGEdgeCount];
  YGValue dimensions[2];
  YGValue minDimensions[2];
  YGValue maxDimensions[2];

  // Yoga specific properties, not compatible with flexbox specification
  float aspectRatio;
} YGStyle;

typedef struct YGConfig {
  bool experimentalFeatures[YGExperimentalFeatureCount + 1];
  bool useWebDefaults;
  float pointScaleFactor;
} YGConfig;

typedef struct YGNode {
  YGStyle style;
  YGLayout layout;
  uint32_t lineIndex;

  YGNodeRef parent;
  YGNodeListRef children;

  struct YGNode *nextChild;

  YGMeasureFunc measure;
  YGBaselineFunc baseline;
  YGPrintFunc print;
  YGConfigRef config;
  void *context;

  bool isDirty;
  bool hasNewLayout;

  YGValue const *resolvedDimensions[2];
} YGNode;

bool YGLayoutNodeInternal(const YGNodeRef node,
                          const float availableWidth,
                          const float availableHeight,
                          const YGDirection parentDirection,
                          const YGMeasureMode widthMeasureMode,
                          const YGMeasureMode heightMeasureMode,
                          const float parentWidth,
                          const float parentHeight,
                          const bool performLayout,
                          const char *reason,
                          const YGConfigRef config);

}  // extern "C"

namespace nu {

namespace {

// Whether a Container is a root CSS node.
inline bool IsRootYGNode(Container* view) {
  return !YGNodeGetParent(view->node()) || !view->GetParent();
}

// Get bounds from the CSS node.
inline RectF GetYGNodeBounds(YGNodeRef node) {
  return RectF(YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node),
               YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));
}

}  // namespace

// static
const char Container::kClassName[] = "Container";

Container::Container() {
  PlatformInit();
}

Container::Container(const char* an_empty_constructor) {
}

Container::~Container() {
  PlatformDestroy();
}

const char* Container::GetClassName() const {
  return kClassName;
}

void Container::Layout() {
  // For child CSS node, tell parent to do the layout.
  if (!IsRootYGNode(this)) {
    static_cast<Container*>(GetParent())->Layout();
    return;
  }

  // So this is a root CSS node, calculate the layout and set bounds.
  SizeF size(GetBounds().size());
  YGNodeCalculateLayout(node(), size.width(), size.height(), YGDirectionLTR);
  SetChildBoundsFromCSS();
}

void Container::BoundsChanged() {
  if (IsRootYGNode(this))
    Layout();
  else
    SetChildBoundsFromCSS();
}

SizeF Container::GetPreferredSize() const {
  float float_max = std::numeric_limits<float>::max();
  YGLayoutNodeInternal(node(), float_max, float_max, YGDirectionLTR,
                       YGMeasureModeAtMost, YGMeasureModeAtMost,
                       float_max, float_max, false, "GetPreferredSize",
                       node()->config);
  return SizeF(node()->layout.measuredDimensions[YGDimensionWidth],
               node()->layout.measuredDimensions[YGDimensionHeight]);
}

float Container::GetPreferredHeightForWidth(float width) const {
  float float_max = std::numeric_limits<float>::max();
  YGLayoutNodeInternal(node(), width, float_max, YGDirectionLTR,
                       YGMeasureModeExactly, YGMeasureModeAtMost, float_max,
                       float_max, false, "GetPreferredHeightForWidth",
                       node()->config);
  return node()->layout.measuredDimensions[YGDimensionHeight];
}

float Container::GetPreferredWidthForHeight(float height) const {
  float float_max = std::numeric_limits<float>::max();
  YGLayoutNodeInternal(node(), float_max, height, YGDirectionLTR,
                       YGMeasureModeAtMost, YGMeasureModeExactly, float_max,
                       float_max, false, "GetPreferredWidthForHeight",
                       node()->config);
  return node()->layout.measuredDimensions[YGDimensionWidth];
}

void Container::AddChildView(View* view) {
  DCHECK(view);
  if (view->GetParent() == this)
    return;
  AddChildViewAt(view, ChildCount());
}

void Container::AddChildViewAt(View* view, int index) {
  DCHECK(view);
  if (view == this || index < 0 || index > ChildCount())
    return;

  if (view->GetParent()) {
    LOG(ERROR) << "The view already has a parent.";
    return;
  }

  view->set_parent(this);
  YGNodeInsertChild(node(), view->node(), index);

  children_.insert(children_.begin() + index, view);
  PlatformAddChildView(view);

  DCHECK_EQ(static_cast<int>(YGNodeGetChildCount(node())), ChildCount());

  Layout();
}

void Container::RemoveChildView(View* view) {
  const auto i(std::find(children_.begin(), children_.end(), view));
  if (i == children_.end())
    return;

  view->set_parent(nullptr);
  YGNodeRemoveChild(node(), view->node());

  PlatformRemoveChildView(view);
  children_.erase(i);

  DCHECK_EQ(static_cast<int>(YGNodeGetChildCount(node())), ChildCount());

  Layout();
}

void Container::SetChildBoundsFromCSS() {
  for (int i = 0; i < ChildCount(); ++i) {
    View* child = ChildAt(i);
    if (child->IsVisible())
      child->SetBounds(GetYGNodeBounds(child->node()));
  }
}

}  // namespace nu
