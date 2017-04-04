// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
// Calculate the size of text according to the view.

#ifndef NATIVEUI_GFX_WIN_TEXT_WIN_H_
#define NATIVEUI_GFX_WIN_TEXT_WIN_H_

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/size_f.h"

namespace nu {

SizeF MeasureText(const base::string16& text, Font* font);
SizeF MeasureText(HDC dc, const base::string16& text, Font* font);

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_TEXT_WIN_H_
