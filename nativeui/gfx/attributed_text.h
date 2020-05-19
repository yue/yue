// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_ATTRIBUTED_TEXT_H_
#define NATIVEUI_GFX_ATTRIBUTED_TEXT_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/text.h"
#include "nativeui/types.h"

namespace nu {

class Font;
class RectF;
class SizeF;

class NATIVEUI_EXPORT AttributedText : public base::RefCounted<AttributedText> {
 public:
  AttributedText(const std::string& text, TextFormat format);
#if defined(OS_WIN)
  AttributedText(base::string16 text, TextFormat format);
#endif

  void SetFormat(TextFormat format);
  const TextFormat& GetFormat() const { return format_; }

  void SetFont(Font* font);
  void SetFontFor(Font* font, int start, int end);
  void SetColor(Color font);
  void SetColorFor(Color font, int start, int end);

  RectF GetBoundsFor(const SizeF& size) const;
  std::string GetText() const;

  // Private: Get one line's size.
  SizeF GetOneLineSize() const;
  float GetOneLineHeight() const;

  NativeAttributedText GetNative() const { return text_; }

 protected:
  virtual ~AttributedText();

 private:
  friend class base::RefCounted<AttributedText>;

  void PlatformUpdateFormat();
  void PlatformSetFontFor(Font* font, int start, int end);
  void PlatformSetColorFor(Color color, int start, int end);

  NativeAttributedText text_;
  TextFormat format_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_ATTRIBUTED_TEXT_H_
