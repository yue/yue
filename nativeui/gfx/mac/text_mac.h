// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_MAC_TEXT_MAC_H_
#define NATIVEUI_GFX_MAC_TEXT_MAC_H_

#include <string>

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/size_f.h"

namespace nu {

// Calculate the size of text.
SizeF MeasureText(const std::string& text, Font* font);

}  // namespace nu

#endif  // NATIVEUI_GFX_MAC_TEXT_MAC_H_
