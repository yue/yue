// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GRAPHICS_FONT_H_
#define NATIVEUI_GRAPHICS_FONT_H_

#include "nativeui/types.h"

namespace nu {

struct Font {
  Font();
  Font(const String& family, int size) : family(family), size(size) {}

  String family;
  int size;
};

Font GetDefaultFont();

}  // namespace nu

#endif  // NATIVEUI_GRAPHICS_FONT_H_
