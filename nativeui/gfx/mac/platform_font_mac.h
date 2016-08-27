// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_MAC_PLATFORM_FONT_MAC_H_
#define NATIVEUI_GFX_MAC_PLATFORM_FONT_MAC_H_

#include <string>

#include "base/mac/scoped_nsobject.h"
#include "nativeui/gfx/platform_font.h"

namespace nu {

NATIVEUI_EXPORT class PlatformFontMac : public PlatformFont {
 public:
  PlatformFontMac();
  PlatformFontMac(const std::string& font_name, int font_size);

  // PlatformFont:
  std::string GetFontName() const override;
  int GetFontSize() const override;
  NativeFont GetNativeFont() const override;

 private:
  ~PlatformFontMac() override;

  base::scoped_nsobject<NSFont> font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_MAC_PLATFORM_FONT_MAC_H_
