// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/painter.h"

#include "nativeui/gfx/pen.h"

namespace nu {

void Painter::DrawString(const String& text, Font font, Color color,
                         const Rect& rect) {
  // We might want to support RTL in future.
  DrawStringWithFlags(text, font, color, rect, TextAlignLeft);
}

}  // namespace nu
