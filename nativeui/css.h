// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CSS_H_
#define NATIVEUI_CSS_H_

#include <string>

typedef struct CSSNode *CSSNodeRef;

namespace nu {

void SetCSSStyle(CSSNodeRef node,
                 const std::string& name,
                 const std::string& value);

}  // namespace nu

#endif  // NATIVEUI_CSS_H_
