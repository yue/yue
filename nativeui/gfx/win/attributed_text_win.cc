// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/attributed_text_win.h"

#include <string>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/win/screen_win.h"

namespace nu {

// Win32 requires specifying font and color when drawing text, so convert format
// to attributes to pass default font and color implicitly.
AttributedText::AttributedText(const std::string& text, TextFormat format)
    : AttributedText(base::UTF8ToUTF16(text),
                     TextAttributes(std::move(format))) {}

AttributedText::AttributedText(const std::string& text, TextAttributes att)
    : AttributedText(base::UTF8ToUTF16(text), std::move(att)) {}

AttributedText::AttributedText(base::string16 text, TextAttributes att) {
  text_ = new AttributedTextImpl;
  text_->text = std::move(text);
  SetFormat(att.ToTextFormat());
  SetFont(att.font.get());
  SetColor(att.color);
}

AttributedText::~AttributedText() {
  delete text_;
}

void AttributedText::PlatformUpdateFormat() {
  text_->format.SetAlignment(ToGdi(format_.align));
  text_->format.SetLineAlignment(ToGdi(format_.valign));
  if (format_.ellipsis)
    text_->format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
  int flags = Gdiplus::StringFormatFlagsMeasureTrailingSpaces;
  if (!format_.wrap)
    flags |= Gdiplus::StringFormatFlagsNoWrap;
  text_->format.SetFormatFlags(flags);
}

void AttributedText::PlatformSetFontFor(Font* font, int start, int end) {
  CHECK(start == 0 && end == -1)
      << "AttributedText.SetFontFor does not work on Windows";
  text_->font = font;
}

void AttributedText::PlatformSetColorFor(Color color, int start, int end) {
  CHECK(start == 0 && end == -1)
      << "AttributedText.SetColorFor does not work on Windows";
  text_->brush.reset(new Gdiplus::SolidBrush(ToGdi(color)));
}

RectF AttributedText::GetBoundsFor(const SizeF& size) const {
  // MeasureString does not take account of the last new line, add a character
  // to make it behave the same with other platforms.
  bool ends_with_newline = text_->text.size() > 0 &&
                           text_->text[text_->text.size() - 1] == L'\n';
  const base::string16& text = ends_with_newline ? text_->text + L"a"
                                                 : text_->text;

  base::win::ScopedGetDC dc(NULL);
  Gdiplus::Graphics graphics(dc);
  // https://stackoverflow.com/questions/1203087/why-is-graphics-measurestring-returning-a-higher-than-expected-number
  graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
  Gdiplus::RectF rect;
  graphics.MeasureString(text.data(), static_cast<int>(text.size()),
                         text_->font->GetNative(), Gdiplus::PointF(0., 0.),
                         &rect);
  float scale_factor = GetScalingFactorFromDPI(::GetDeviceCaps(dc, LOGPIXELSX));
  return RectF(0., 0., rect.Width / scale_factor, rect.Height / scale_factor);
}

std::string AttributedText::GetText() const {
  return base::UTF16ToUTF8(text_->text);
}

}  // namespace nu
