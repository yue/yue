// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_WIDGET_UTIL_H_
#define NATIVEUI_GTK_WIDGET_UTIL_H_

#include "nativeui/gfx/geometry/size.h"
#include "nativeui/types.h"

namespace nu {

Size GetPreferredSizeForWidget(NativeView widget);

}  // namespace nu

#endif  // NATIVEUI_GTK_WIDGET_UTIL_H_
