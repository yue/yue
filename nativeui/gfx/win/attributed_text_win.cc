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

AttributedText::AttributedText(const std::string& text, TextFormat format) {
  text_ = new AttributedTextImpl;
  text_->text = base::UTF8ToUTF16(text);
  SetFormat(std::move(format));
  // Set default font and color.
  TextAttributes default_attributes;
  SetFont(default_attributes.font.get());
  SetColor(default_attributes.color);
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
  // https://stackoverflow.com/questions/1203087/why-is-graphics-measurestring-returning-a-higher-than-expected-number
  base::win::ScopedGetDC dc(NULL);
  Gdiplus::Graphics graphics(dc);
  graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
  Gdiplus::RectF rect;
  graphics.MeasureString(text_->text.data(),
                         static_cast<int>(text_->text.size()),
                         text_->font->GetNative(),
                         Gdiplus::PointF(0., 0.),
                         &rect);
  float scale_factor = GetScalingFactorFromDPI(::GetDeviceCaps(dc, LOGPIXELSX));
  return RectF(0., 0., rect.Width / scale_factor, rect.Height / scale_factor);
}

std::string AttributedText::GetText() const {
  return base::UTF16ToUTF8(text_->text);
}

}  // namespace nu
