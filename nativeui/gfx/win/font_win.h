// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_FONT_WIN_H_
#define NATIVEUI_GFX_WIN_FONT_WIN_H_

#include <string>

#include "nativeui/gfx/font.h"
#include "nativeui/gfx/win/gdiplus.h"

namespace nu {

NATIVEUI_EXPORT class FontWin : public Font {
 public:
  FontWin(const base::string16& font_name, int font_size);

  // Font:
  std::string GetName() const override;
  int GetSize() const override;
  NativeFont GetNative() const override;

 private:
  ~FontWin() override;

  Gdiplus::FontFamily font_family_;
  Gdiplus::Font font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_FONT_WIN_H_
