// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_ATTRIBUTED_TEXT_WIN_H_
#define NATIVEUI_GFX_WIN_ATTRIBUTED_TEXT_WIN_H_

#include <memory>

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

struct AttributedTextImpl {
  base::string16 text;
  scoped_refptr<Font> font;
  std::unique_ptr<Gdiplus::SolidBrush> brush;
  // https://stackoverflow.com/questions/1203087/why-is-graphics-measurestring-returning-a-higher-than-expected-number
  Gdiplus::StringFormat format = Gdiplus::StringFormat::GenericTypographic();
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_ATTRIBUTED_TEXT_WIN_H_
