// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_TEXT_H_
#define NATIVEUI_GFX_TEXT_H_

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/size_f.h"

namespace nu {

// Text alignment when drawing text.
enum class TextAlign {
  Start,
  Center,
  End,
};

// Attributes used for drawing the text.
struct NATIVEUI_EXPORT TextAttributes {
  TextAttributes();
  TextAttributes(Font* font, Color color, TextAlign align, TextAlign valign);
  ~TextAttributes();

  scoped_refptr<Font> font;
  Color color;
  TextAlign align;
  TextAlign valign;
};

// Result of text measurement.
struct TextMetrics {
  SizeF size;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_TEXT_H_
