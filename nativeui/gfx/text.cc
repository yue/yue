// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/text.h"

#include "nativeui/gfx/font.h"
#include "nativeui/state.h"

namespace nu {

TextAttributes::TextAttributes()
    : font(nu::State::GetCurrent()->GetDefaultFont()),
      color(GetSystemColor(SystemColor::Text)),
      align(TextAlign::Start),
      valign(TextAlign::Start) {
}

TextAttributes::TextAttributes(Font* font, Color color, TextAlign align,
                               TextAlign valign)
    : font(font), color(color), align(align), valign(valign) {
}

TextAttributes::~TextAttributes() {
}

}  // namespace nu
