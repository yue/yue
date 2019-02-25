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

// Options for drawing text.
struct NATIVEUI_EXPORT TextFormat {
  TextAlign align = TextAlign::Start;
  TextAlign valign = TextAlign::Start;
  bool wrap = true;
  bool ellipsis = false;
};

// Attributes used for drawing the text.
struct NATIVEUI_EXPORT TextAttributes : public TextFormat {
  // Full constructor.
  TextAttributes(Font* font,
                 Color color,
                 TextAlign align = TextAlign::Start,
                 TextAlign valign = TextAlign::Start,
                 bool wrap = true,
                 bool ellipsis = false);
  ~TextAttributes();

  // Helpers.
  TextAttributes();
  explicit TextAttributes(Font* font);
  explicit TextAttributes(Color color);

  scoped_refptr<Font> font;
  Color color;
};

// Result of text measurement.
struct TextMetrics {
  SizeF size;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_TEXT_H_
