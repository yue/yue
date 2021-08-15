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
  AttributedText(const std::string& text, TextAttributes att);
#if defined(OS_WIN)
  AttributedText(std::wstring text, TextAttributes att);
#endif

  void SetFormat(TextFormat format);
  const TextFormat& GetFormat() const { return format_; }

  void SetFont(scoped_refptr<Font> font);
  void SetFontFor(scoped_refptr<Font> font, int start, int end);
  void SetColor(Color color);
  void SetColorFor(Color color, int start, int end);
  void Clear();

  RectF GetBoundsFor(const SizeF& size) const;

  void SetText(const std::string& text);
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
  void PlatformSetFontFor(scoped_refptr<Font> font, int start, int end);
  void PlatformSetColorFor(Color color, int start, int end);

  NativeAttributedText text_;
  TextFormat format_;
};

}  // namespace nu

#endif  // NATIVEUI_GFX_ATTRIBUTED_TEXT_H_
