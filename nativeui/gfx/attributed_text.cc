// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/attributed_text.h"

#include <float.h>  // needed for FLT_MAX on arm64 macOS

#include <utility>

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/rect_f.h"

namespace nu {

namespace {

inline bool RangeInvalid(int start, int end) {
  return start < 0 || (end >= 0 && end <= start);
}

}  // namespace

// The system does not specify default system font and color for AttributedText
// so convert format to attributes to pass default font and color implicitly.
AttributedText::AttributedText(const std::string& text, TextFormat format)
    : AttributedText(text, TextAttributes(std::move(format))) {}

void AttributedText::SetFormat(TextFormat format) {
  format_ = std::move(format);
  PlatformUpdateFormat();
}

void AttributedText::SetFont(scoped_refptr<Font> font) {
  SetFontFor(std::move(font), 0, -1);
}

void AttributedText::SetFontFor(scoped_refptr<Font> font, int start, int end) {
  if (RangeInvalid(start, end))
    return;
  PlatformSetFontFor(std::move(font), start, end);
}

void AttributedText::SetColor(Color color) {
  SetColorFor(color, 0, -1);
}

void AttributedText::SetColorFor(Color color, int start, int end) {
  if (RangeInvalid(start, end))
    return;
  PlatformSetColorFor(color, start, end);
}

void AttributedText::Clear() {
  TextAttributes attrs;
  SetFont(std::move(attrs.font));
  SetColor(attrs.color);
}

SizeF AttributedText::GetOneLineSize() const {
  return GetBoundsFor(SizeF(FLT_MAX, FLT_MAX)).size();
}

float AttributedText::GetOneLineHeight() const {
  return GetOneLineSize().height();
}

}  // namespace nu
