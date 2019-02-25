// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/text.h"

#include "nativeui/gfx/font.h"
#include "nativeui/system.h"

namespace nu {

TextAttributes::TextAttributes(Font* font, Color color, TextAlign align,
                               TextAlign valign, bool wrap, bool ellipsis)
    : TextFormat({align, valign, wrap, ellipsis}),
      font(font), color(color) {}

TextAttributes::TextAttributes()
    : TextAttributes(System::GetDefaultFont(),
                     System::GetColor(System::Color::Text)) {}

TextAttributes::TextAttributes(Font* font)
    : TextAttributes(font, System::GetColor(System::Color::Text)) {}

TextAttributes::TextAttributes(Color color)
    : TextAttributes(System::GetDefaultFont(), color) {}

TextAttributes::~TextAttributes() {
}

}  // namespace nu
