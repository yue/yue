// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_FONT_H_
#define NATIVEUI_GFX_FONT_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

class PlatformFont;

// Font provides a wrapper around an underlying font. Copy and assignment
// operators are allowed and cheap.
NATIVEUI_EXPORT class Font {
 public:
  // Creates a font with the default name and style.
  Font();

  // Creates a font with the specified name in UTF-8 and size.
  Font(const std::string& family, int size);

  ~Font();

  // Returns the specified font name in UTF-8.
  std::string GetFontName() const;

  // Returns the actually used font name in UTF-8.
  std::string GetActualFontName() const;

  // Returns the font size.
  int GetFontSize() const;

#if defined(OS_WIN) || defined(OS_MACOSX) || defined(OS_IOS)
  // Returns the native font handle.
  // Lifetime lore:
  // Windows: This handle is owned by the Font object, and should not be
  //          destroyed by the caller.
  // Mac:     The object is owned by the system and should not be released.
  NativeFont GetNativeFont() const;
#endif

  // Raw access to the underlying platform font implementation. Can be
  // static_cast to a known implementation type if needed.
  PlatformFont* platform_font() const { return platform_font_.get(); }

 private:
  // Wrapped platform font implementation.
  scoped_refptr<PlatformFont> platform_font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_FONT_H_
