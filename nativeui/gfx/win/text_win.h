// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
// Calculate the size of text according to the view.

#ifndef NATIVEUI_GFX_WIN_TEXT_WIN_H_
#define NATIVEUI_GFX_WIN_TEXT_WIN_H_

#include "nativeui/gfx/text.h"

namespace nu {

class BaseView;

SizeF MeasureText(const nu::BaseView* view, const Font& font,
                  const String& text);

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_TEXT_WIN_H_
