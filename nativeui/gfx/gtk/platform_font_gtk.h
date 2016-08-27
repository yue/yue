// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PLATFORM_FONT_GTK_H_
#define NATIVEUI_GFX_GTK_PLATFORM_FONT_GTK_H_

#include "nativeui/gfx/platform_font.h"

namespace nu {

NATIVEUI_EXPORT class PlatformFontGtk : public PlatformFont {
 public:
  explicit PlatformFontGtk(PangoFontDescription* font);
  PlatformFontGtk(const std::string& font_name, int font_size);

  // PlatformFont:
  std::string GetFontName() const override;
  int GetFontSize() const override;

 private:
  ~PlatformFontGtk() override;

  PangoFontDescription* font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PLATFORM_FONT_GTK_H_
