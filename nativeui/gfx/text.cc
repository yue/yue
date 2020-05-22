// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/text.h"

#include <utility>

#include "nativeui/gfx/font.h"

namespace nu {

TextAttributes::TextAttributes(Font* font, Color color, TextAlign align,
                               TextAlign valign, bool wrap, bool ellipsis)
    : TextFormat({align, valign, wrap, ellipsis}),
      font(font), color(color) {}

TextAttributes::TextAttributes()
    : TextAttributes(Font::Default(), Color::Get(Color::Name::Text)) {}

TextAttributes::TextAttributes(TextFormat format)
    : TextFormat(std::move(format)),
      font(Font::Default()),
      color(Color::Get(Color::Name::Text)) {}

TextAttributes::TextAttributes(Font* font)
    : TextAttributes(font, Color::Get(Color::Name::Text)) {}

TextAttributes::TextAttributes(Color color)
    : TextAttributes(Font::Default(), color) {}

TextAttributes::TextAttributes(const TextAttributes&) = default;
TextAttributes::TextAttributes(TextAttributes&&) = default;

TextAttributes::~TextAttributes() = default;

TextFormat TextAttributes::ToTextFormat() const {
  return {align, valign, wrap, ellipsis};
}

}  // namespace nu
