// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/attributed_text.h"

#include "nativeui/gfx/font.h"

namespace nu {

namespace {

inline bool RangeInvalid(int start, int end) {
  return start < 0 || (end >= 0 && end <= start);
}

}  // namespace

void AttributedText::SetFont(Font* font) {
  SetFontFor(font, 0, -1);
}

void AttributedText::SetFontFor(Font* font, int start, int end) {
  if (RangeInvalid(start, end))
    return;
  PlatformSetFontFor(font, start, end);
  // Do a AddRef/ReleaseRef to prevent leak if the font is a floating pointer.
  scoped_refptr<Font>{font};
}

void AttributedText::SetColor(Color color) {
  SetColorFor(color, 0, -1);
}

void AttributedText::SetColorFor(Color color, int start, int end) {
  if (RangeInvalid(start, end))
    return;
  PlatformSetColorFor(color, start, end);
}

}  // namespace nu
