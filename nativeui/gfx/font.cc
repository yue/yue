// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/font.h"

#include "nativeui/app.h"

namespace nu {

Font* Font::Derive(float size_delta, Weight weight, Style style) const {
  return new Font(GetName(), GetSize() + size_delta, weight, style);
}

}  // namespace nu
