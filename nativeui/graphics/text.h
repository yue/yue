// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GRAPHICS_TEXT_H_
#define NATIVEUI_GRAPHICS_TEXT_H_

#include "nativeui/graphics/font.h"
#include "ui/gfx/geometry/size_f.h"

namespace nu {

gfx::SizeF MeasureText(const Font& font, const String& text);

}  // namespace nu

#endif  // NATIVEUI_GRAPHICS_TEXT_H_
