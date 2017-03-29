// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_UTIL_YOGA_H_
#define NATIVEUI_UTIL_YOGA_H_

#include <string.h>

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

// Extended Yoga APIs.
namespace nu {

inline void YGConfigCopy(YGConfigRef target, YGConfigRef source) {
  memcpy(target, source, sizeof(YGConfig));
}

}  // namespace nu

#endif  // NATIVEUI_UTIL_YOGA_H_
