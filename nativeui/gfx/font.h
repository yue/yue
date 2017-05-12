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

class NATIVEUI_EXPORT Font : public base::RefCounted<Font> {
 public:
  // Standard font weights as used in Pango and Windows. The values must match
  // https://msdn.microsoft.com/en-us/library/system.windows.fontweights(v=vs.110).aspx
  enum class Weight {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Normal = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900,
  };

  // The following constants indicate the font style.
  enum class Style {
    Normal = 0,
    Italic = 1,
  };

  // Create default system UI font.
  Font();
  // Create a Font implementation with the specified |name|
  // (encoded in UTF-8), DIP |size|, |weight| and |style|.
  Font(const std::string& name, float size, Weight weight, Style style);

  // Return the specified font name in UTF-8.
  std::string GetName() const;

  // Return the font size in pixels.
  float GetSize() const;

  // Return the font weight.
  Weight GetWeight() const;

  // Return the font style.
  Style GetStyle() const;

  // Return the native font handle.
  NativeFont GetNative() const;

 protected:
  virtual ~Font();

 private:
  friend class base::RefCounted<Font>;

  NativeFont font_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_FONT_H_
