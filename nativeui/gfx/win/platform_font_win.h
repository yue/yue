// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_PLATFORM_FONT_WIN_H_
#define NATIVEUI_GFX_WIN_PLATFORM_FONT_WIN_H_

#include <string>

#include "nativeui/gfx/platform_font.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

NATIVEUI_EXPORT class PlatformFontWin : public PlatformFont {
 public:
  PlatformFontWin(const base::string16& font_name, int font_size);

  // PlatformFont:
  std::string GetFontName() const override;
  int GetFontSize() const override;
  NativeFont GetNativeFont() const override;

 private:
  Gdiplus::FontFamily font_family_;
  Gdiplus::Font font_;

  DISALLOW_COPY_AND_ASSIGN(PlatformFontWin);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_PLATFORM_FONT_WIN_H_
