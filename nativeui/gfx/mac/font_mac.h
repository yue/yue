// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_MAC_FONT_MAC_H_
#define NATIVEUI_GFX_MAC_FONT_MAC_H_

#include <string>

#include "base/mac/scoped_nsobject.h"
#include "nativeui/gfx/font.h"

namespace nu {

NATIVEUI_EXPORT class FontMac : public Font {
 public:
  FontMac();
  FontMac(const std::string& font_name, int font_size);

  // Font:
  std::string GetName() const override;
  int GetSize() const override;
  NativeFont GetNative() const override;

 private:
  ~FontMac() override;

  base::scoped_nsobject<NSFont> font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_MAC_FONT_MAC_H_
