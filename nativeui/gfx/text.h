// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_TEXT_H_
#define NATIVEUI_GFX_TEXT_H_

#include "nativeui/gfx/color.h"

namespace nu {

class Font;

// Text alignment when drawing text.
enum class TextAlign {
  Start,
  Center,
  End,
};

// Attributes used for drawing the text.
struct NATIVEUI_EXPORT TextAttributes {
  TextAttributes();

  Font* font;
  Color color;
  TextAlign align;
  TextAlign valign;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_TEXT_H_
