// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_PLATFORM_FONT_H_
#define NATIVEUI_GFX_PLATFORM_FONT_H_

#include <string>

#include "nativeui/gfx/font.h"
#include "nativeui/types.h"

namespace nu {

NATIVEUI_EXPORT class PlatformFont : public base::RefCounted<PlatformFont> {
 public:
  // Creates an appropriate PlatformFont implementation.
  static PlatformFont* CreateDefault();
  // Creates a PlatformFont implementation with the specified |font_name|
  // (encoded in UTF-8) and |font_size| in pixels.
  static PlatformFont* CreateFromNameAndSize(const std::string& font_name,
                                             int font_size);

  // Returns the specified font name in UTF-8.
  virtual std::string GetFontName() const = 0;

  // Returns the font size in pixels.
  virtual int GetFontSize() const = 0;

#if defined(OS_WIN) || defined(OS_MACOSX) || defined(OS_IOS)
  // Returns the native font handle.
  virtual NativeFont GetNativeFont() const = 0;
#endif

 protected:
  PlatformFont() {}
  virtual ~PlatformFont() {}

 private:
  friend class base::RefCounted<PlatformFont>;

  DISALLOW_COPY_AND_ASSIGN(PlatformFont);
};

}  // namespace nu

#endif  // NATIVEUI_GFX_PLATFORM_FONT_H_
