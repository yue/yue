// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include "nativeui/state.h"

namespace nu {

// static
Font* Font::Default() {
  auto& default_font = State::GetCurrent()->default_font();
  if (!default_font)
    default_font = new Font;
  return default_font.get();
}

Font* Font::Derive(float size_delta, Weight weight, Style style) const {
  return new Font(GetName(), GetSize() + size_delta, weight, style);
}

}  // namespace nu
