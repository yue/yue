// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/attributed_text.h"

#include <utility>

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/rect_f.h"

namespace nu {

namespace {

inline bool RangeInvalid(int start, int end) {
  return start < 0 || (end >= 0 && end <= start);
}

}  // namespace

void AttributedText::SetFormat(TextFormat format) {
  format_ = std::move(format);
  PlatformUpdateFormat();
}

void AttributedText::SetFont(Font* font) {
  SetFontFor(font, 0, -1);
}

void AttributedText::SetFontFor(Font* font, int start, int end) {
  if (RangeInvalid(start, end))
    return;
  PlatformSetFontFor(font, start, end);
  // Do a AddRef/ReleaseRef to prevent leak if the font is a floating pointer.
  base::WrapRefCounted(font);
}

void AttributedText::SetColor(Color color) {
  SetColorFor(color, 0, -1);
}

void AttributedText::SetColorFor(Color color, int start, int end) {
  if (RangeInvalid(start, end))
    return;
  PlatformSetColorFor(color, start, end);
}

SizeF AttributedText::GetOneLineSize() const {
  return GetBoundsFor(SizeF(FLT_MAX, FLT_MAX)).size();
}

float AttributedText::GetOneLineHeight() const {
  return GetOneLineSize().height();
}

}  // namespace nu
