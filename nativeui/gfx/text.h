// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_TEXT_H_
#define NATIVEUI_GFX_TEXT_H_

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/size_f.h"

namespace nu {

// Calculate the size of text.
SizeF MeasureText(const Font& font, const String& text);

}  // namespace nu

#endif  // NATIVEUI_GFX_TEXT_H_
