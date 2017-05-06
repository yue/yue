// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_UTIL_YOGA_UTIL_H_
#define NATIVEUI_UTIL_YOGA_UTIL_H_

#include <string>

typedef struct YGNode *YGNodeRef;

namespace nu {

void SetYogaProperty(YGNodeRef node, const std::string& key, float value);
void SetYogaProperty(YGNodeRef node,
                     const std::string& key,
                     const std::string& value);

}  // namespace nu

#endif  // NATIVEUI_UTIL_YOGA_UTIL_H_
